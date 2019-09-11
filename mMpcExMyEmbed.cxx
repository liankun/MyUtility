#include "mMpcExMyEmbed.h"
#include "PHCompositeNode.h"
#include "PHNodeIterator.h"
#include "PHIODataNode.h"
#include "Fun4AllInputManager.h"
#include "Fun4AllDstInputManager.h"
#include "Fun4AllReturnCodes.h"
#include "recoConsts.h"
#include "getClass.h"
#include "MpcExRawHit.h"
#include "MpcExEventHeader.h"
#include "TMpcExHitContainer.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExGeaHitContainer.h"
#include "TMpcExCalib.h"
#include "TMpcExGeaHit.h"
#include "mMpcExApplyCalibrations.h"
#include "math.h"
#include <algorithm>

using namespace std;
using namespace findNode;

const int MAXERR = 1.e4;
const int ADCOVERFLOW = 255;

mMpcExMyEmbed::mMpcExMyEmbed()
{
  ThisName = "MPCEXEMBED";

  hVertexDiff = NULL;
  hMpcExDiff = NULL;
  hMpcExNHit = NULL;
  hEmbedHighADC = NULL;
  hEmbedLowADC = NULL;
  for (int i = 0; i < 3; i++) {
    hMpcExAdcHL[i] = NULL;
  }

  fSigNodeName = "SIM";  // default
  fBgNodeName = "TOP";

  fMpcExGeaHitCont = NULL;
  fDice = NULL;
  fMpcExRawHit_Bg = NULL;
  fPHGlobal_Sig = NULL;
  fPHGlobal_Bg = NULL;
  fMpcExCalibCont = NULL;
  mpcexmap = NULL;
  fDebug = 0;
  fQAFlag = 0;
  IEvt = 0;
  fVertexDiff = 6.0;
  fNErr = 0;

  recoConsts *myrc = recoConsts::instance();

  // Calibrations mode must be COMPLETE as this module will assume 
  // that all calibrations are being applied to the data 
  calibMode = (mMpcExApplyCalibrations::Mode) myrc->get_IntFlag("MPCEXCALIBMODE",mMpcExApplyCalibrations::COMPLETE);  
  disable_MPV_layer_adjust = myrc->get_IntFlag("MPCEX_NO_LAYER_MPV_ADJUST",0x0); 


}

mMpcExMyEmbed::~mMpcExMyEmbed()
{
  if (fDice) delete fDice;
  if (exo_Sig) delete exo_Sig;
  if (exo_Bg) delete exo_Bg;
  if (exo_Em) delete exo_Em;
}



int mMpcExMyEmbed::InitRun(PHCompositeNode *topNode)
{
  mpcexmap = MpcExMapper::instance();
  fDice = new TRandom3(0);
  if (fQAFlag) {
    Fun4AllServer *se = Fun4AllServer::instance();

    int nbins = 100;
    hVertexDiff = new TH1D("hVertexDiff", "Vertex Diff. btw. Real DST and simDST", nbins, -4, 4);
    hVertexDiff->GetXaxis()->SetTitle("Vertex Diff(cm)");
    se->registerHisto(hVertexDiff);

    hMpcExDiff = new TH1D("hMpcExDiff", "MpcEx HitSum Diff. btw. Real DST and simDST", nbins, -10, 10);
    hMpcExDiff->GetXaxis()->SetTitle("HitSum Diff");
    se->registerHisto(hMpcExDiff);

    hMpcExNHit = new TH1D("hMpcExNHit", "MpcEx NHit Diff. btw. Real DST and simDST", nbins, -10, 10);
    hMpcExNHit->GetXaxis()->SetTitle("NHit Diff");
    se->registerHisto(hMpcExNHit);

    for (int i = 0; i < 3; i++) {
      hMpcExAdcHL[i] = new TH2F(Form("hMpcExAdcHL%d", i), "MpcEx ADC High vs Low", nbins, 0, ADCOVERFLOW+20, nbins, 0, ADCOVERFLOW+20);
      hMpcExAdcHL[i]->GetXaxis()->SetTitle("Low ADC");
      hMpcExAdcHL[i]->GetYaxis()->SetTitle("High ADC");
      se->registerHisto(hMpcExAdcHL[i]);
    }

    int nMinipad = 128*24*16;
    hEmbedHighADC = new TH2F("hEmbedHighADC", "High Embedded ADC", nMinipad, 0, nMinipad, ADCOVERFLOW+20, 0, ADCOVERFLOW+20);
    hEmbedHighADC->GetXaxis()->SetTitle("Minipad");
    hEmbedHighADC->GetYaxis()->SetTitle("High ADC");
    se->registerHisto(hEmbedHighADC);

    hEmbedLowADC = new TH2F("hEmbedLowADC", "Low Embedded ADC", nMinipad, 0, nMinipad, ADCOVERFLOW+20, 0, ADCOVERFLOW+20);
    hEmbedLowADC->GetXaxis()->SetTitle("Minipad");
    hEmbedLowADC->GetYaxis()->SetTitle("Low ADC");
    se->registerHisto(hEmbedLowADC);

    exo_Sig = new Exogram("exo_Sig", "exo_Sig", 900, -24, 24, 900, -24, 24, 8, -0.5, 7.5);
    exo_Bg = new Exogram("exo_Bg", "exo_Bg", 900, -24, 24, 900, -24, 24, 8, -0.5, 7.5);
    exo_Em = new Exogram("exo_Em", "exo_Em", 900, -24, 24, 900, -24, 24, 8, -0.5, 7.5);

  } // QAFlag

  return EVENT_OK;
}



int mMpcExMyEmbed::process_event(PHCompositeNode* topNode)
{
  if (fDebug) cout << "Event : " << IEvt << endl;
  IEvt++;
  if (GetNode(topNode) == ABORTEVENT) return ABORTEVENT;
  if (EventCheck(topNode) == ABORTEVENT) return ABORTEVENT;
  EmbedMpcEx();

  return EVENT_OK;
}



int mMpcExMyEmbed::EmbedMpcEx()
{
  if (fDebug) cout << "EmbedMpcEx" << endl;
  int nhit_Bg = fMpcExRawHit_Bg->getnhits();
  int nhit_Sig = fMpcExGeaHitCont->size();

  std::vector<unsigned int> raw;
  raw.clear(); 

  int BgCount = 0;
  int EmCount = 0;
  int SigCount = 0;
  int OverlapCount = 0;
  int BgSum = 0;
  int SigSum = 0;
  int OverSum = 0;
  int EmSum = 0;

  int BgSum_low = 0;
  int SigSum_low = 0;
  int OverSum_low = 0;
  int EmSum_low = 0;

  if (fQAFlag) {
    exo_Sig->Reset();
    exo_Bg->Reset();
    exo_Em->Reset();
  }
  // Bg loop
  for (int idx = 0; idx < nhit_Bg; idx++){
    unsigned short Bg_low = fMpcExRawHit_Bg->getladc(idx);
    unsigned short Bg_high =  fMpcExRawHit_Bg->gethadc(idx);
    unsigned int key = fMpcExRawHit_Bg->getOnlineKey(idx);
    // Make no judgements here!
    // All original background hits should continue going forward
    //TMpcExCalib *Calib = fMpcExCalibCont->get(key);
    //int badh = Calib->high_dead_hot_status();
    //int badl = Calib->low_dead_hot_status();
    //if (badh != 0 && badl != 0) continue;
    //if (badh != 0) Bg_high = 0;
    //if (badl != 0) Bg_low = 0;
    BgCount++;
    BgSum += Bg_high;
    BgSum_low += Bg_low;
    if (fQAFlag) {
      exo_Bg->FillEx(key, Bg_high);
      //if (badl == 0 && badh == 0) hMpcExAdcHL[1]->Fill(Bg_low, Bg_high);
    }
    unsigned short adc = (Bg_high << 8) + Bg_low;
    unsigned int val = key << 16;
    val |= adc;
    raw.push_back(val);
  }

  std::sort(raw.begin(), raw.end());

  // Sig loop
  for (int ihit = 0; ihit < nhit_Sig; ihit++) {
    TMpcExGeaHit *geahit = fMpcExGeaHitCont->getHit(ihit);
    unsigned int key = geahit->key();
    TMpcExCalib *Calib = fMpcExCalibCont->get(key);
    double energy = geahit->e()*fDice->Gaus(1.0,Calib->get_smear());

    int arm = mpcexmap->get_arm(key);
    int badh = Calib->high_dead_hot_status();
    int badl = Calib->low_dead_hot_status();
    
    
    if(calibMode==mMpcExApplyCalibrations::COMPLETE_FIXED_MC_PERFECT){
      badh = 0;
      badl = 0;
    }
    
    // skip bad channels here
    // NOTE: If the low has a dead/hot code of two it is still good for 
    // large ADC values (above the high). We will continue and 
    // mMpcExApplyCalibrations will sort it out. 
    if ( (badh != 0) && ((badl != 0) && (badl!=2)) ) continue;

    double mip_sensor = Calib->get_mip_in_sensor();
    double mip_corr = Calib->get_minipad_mip_correction();


    double mip = mip_sensor*mip_corr;
    if(Calib->get_smear_scale()>0) mip /= Calib->get_smear_scale();
    if(Calib->get_mip_correction()>0) mip /= Calib->get_mip_correction();


    if ((calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_MC_PERFECT)&&(calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_MC)&&
	(calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_REALPED)) mip = mip/Calib->get_mip_correction();
    int layer = mpcexmap->get_layer(key);

    if (mip <= 0 && (calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_MC_PERFECT)&&(calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_MC)&&(calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_REALPED)) {
      //bad calibration should ignore for complete calibration
      continue;
//      if (layer == 0 || layer == 1) mip = MpcExConstants::FIXED_MIP_L01;
//      else mip = MpcExConstants::FIXED_MIP_L27;
    }
    double hlratio = 1.0/Calib->get_high_low_ratio(); // what is in DB is inverted!
    double ped_low = Calib->low_pedestal();
    double ped_high = Calib->high_pedestal();
    double ped_sig_low = Calib->low_pedestal_width();
    double ped_sig_high = Calib->high_pedestal_width();
    
    //for hijing embedding,we should use the fixed constants
    if(calibMode==mMpcExApplyCalibrations::COMPLETE_FIXED_MC_PERFECT || calibMode==mMpcExApplyCalibrations::COMPLETE_FIXED_MC || calibMode==mMpcExApplyCalibrations::COMPLETE_FIXED_REALPED){
      if (layer == 0 || layer == 1) mip = MpcExConstants::FIXED_MIP_L01;
      else mip = MpcExConstants::FIXED_MIP_L27; 
      hlratio = MpcExConstants::FIXED_HL_RATIO;
      //COMPLETE_FIXED_REALPED will use data's pedestal
      if(calibMode!=mMpcExApplyCalibrations::COMPLETE_FIXED_REALPED){
        ped_low = MpcExConstants::FIXED_LOW_PEDESTAL;
	ped_high = MpcExConstants::FIXED_HIGH_PEDESTAL;
	ped_sig_low = MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH;
	ped_sig_high = MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH;
      }
    }

    int idx_bg = FindHit(key, raw);

    // MPV layer adjustment
    double mpv_layer_adjust = MpcExConstants::MIP_IN_keV; 
    if(!disable_MPV_layer_adjust) {
      if(Calib->get_mip_layer_mpv()>0.0) mpv_layer_adjust = Calib->get_mip_layer_mpv(); 
    }

    // Is this raw hit already in the list, or is it new? 

    if (idx_bg < 0) {  // New hit

      // Calculate the signal ADC values 
      // Apply DCM zero suppression
      unsigned short Sig_low = 0.0; 
      unsigned short Sig_high = 0.0; 
      int zh = 0; 
      int zl = 0; 

      if ( (badh == 0) && (ped_high>0.0) && (ped_sig_high>0.0) ){
	Sig_high = MpcExConstants::make_adc(mip, ped_high, ped_sig_high, 
					    (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
 	zh = ZeroSup(Sig_high, 0, arm, ped_high, ped_sig_high);
      }

      if ( ((badl == 0) || (badl == 2)) && (ped_low>0.0) && (ped_sig_low>0.0) && (hlratio>0.0) ){
	Sig_low = MpcExConstants::make_adc(mip/hlratio, ped_low, ped_sig_low, 
					   (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
        zl = ZeroSup(Sig_low, 1, arm, ped_low, ped_sig_low);
      }

      // Bail out if nothing survives
      if ( (zh == 0) && (zl == 0) ) continue;

      SigCount++;
      SigSum += Sig_high;
      SigSum_low += Sig_low;
      if (fQAFlag) {
        exo_Sig->FillEx(key, Sig_high);
        if (badl == 0 && badh == 0) hMpcExAdcHL[0]->Fill(Sig_low, Sig_high);
      }

      unsigned short adc = (Sig_high << 8) + Sig_low;
      unsigned int val = key << 16;
      val |= adc;
      raw.push_back(val);

      // need to re-sort after adding value
      std::sort(raw.begin(), raw.end());

    }
    else if(idx_bg >= 0) { // Overlapping hit

      OverlapCount++;
      SigCount++;

      // We have an overlapping hit, so we have to be careful here. 
      // If there is a valid ADC value for the overlapping hit then we do not 
      // digitize the signal hit to be added with a pedestal and pedestal fluctuations. 
      // However, if there is NO existing hit (ADC==0) then we need to add in the pedestal. 

      unsigned int Bg_high = (raw[idx_bg] & 0x0000FF00) >> 8;
      unsigned int Bg_low = raw[idx_bg] & 0x000000FF; 

      unsigned short Sig_low = 0.0; 
      unsigned short Sig_high = 0.0; 
      //not work for perfect detector
      if ( (badh == 0) && (ped_high>0.0) && (ped_sig_high>0.0)){
	if(Bg_high > 0) 
	  Sig_high = MpcExConstants::make_adc(mip, 0.0, 0.0, 
					    (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
	else
	  Sig_high = MpcExConstants::make_adc(mip, ped_high, ped_sig_high, 
					    (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
	  
      }

      if ( ((badl == 0) || (badl == 2)) && (ped_low>0.0) && (ped_sig_low>0.0)){
	if(Bg_low>0)
	  Sig_low = MpcExConstants::make_adc(mip/hlratio, 0.0, 0.0, 
					   (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
	else
	  Sig_low = MpcExConstants::make_adc(mip/hlratio, ped_low, ped_sig_low, 
					   (energy*1000000)*(mpv_layer_adjust/MpcExConstants::MIP_IN_keV), fDice, NULL);
	  
      }
      SigSum += Sig_high;
      SigSum_low += Sig_low;

      if (fQAFlag) exo_Sig->FillEx(key, Sig_high);

      unsigned short Embed_low = Sig_low + Bg_low;
      unsigned short Embed_high = Sig_high + Bg_high;

      if (Embed_low >= ADCOVERFLOW) {
        OverSum_low += Embed_low - ADCOVERFLOW;
        Embed_low = ADCOVERFLOW;
      }

      if (Embed_high >= ADCOVERFLOW) {
        OverSum += Embed_high - ADCOVERFLOW;
        Embed_high = ADCOVERFLOW;
      }

      unsigned short adc = (Embed_high << 8) + Embed_low;
      unsigned int val = key << 16;

      val |= adc;
      raw[idx_bg] = val;

    }

  } // ihit loop

  // refill the raw hits
  fMpcExRawHit_Bg->fillfromvector(raw);

  int nhit_embed = fMpcExRawHit_Bg->getnhits();

  for (int ihit = 0; ihit < nhit_embed; ihit++) {
    int hadc = fMpcExRawHit_Bg->gethadc(ihit);
    int ladc = fMpcExRawHit_Bg->getladc(ihit);
    unsigned int key = fMpcExRawHit_Bg->getOnlineKey(ihit);
    TMpcExCalib *Calib = fMpcExCalibCont->get(key);
    int badh = Calib->high_dead_hot_status();
    int badl = Calib->low_dead_hot_status();
    if (badh != 0 && badl != 0) continue;
    EmCount++;
    EmSum += hadc;
    EmSum_low += ladc;

    if (fQAFlag) {
      if (badh == 0 && badl == 0) {
        hMpcExAdcHL[2]->Fill(ladc, hadc);
        hEmbedHighADC->Fill(key, hadc);
        hEmbedLowADC->Fill(key, ladc);
      }
      exo_Em->FillEx(key, hadc);
    }

  } // ihit

  if (fDebug) {
    cout << "mMpcExMyEmbedding :" << endl;
    cout << "nhit_sig : " << SigCount << endl;
    cout << "nhit_before : " << BgCount << endl;
    cout << "nhit_overlap : " << OverlapCount << endl;
    cout << "nhit_after : " << EmCount << endl;
    cout << "nhit_err : " << EmCount - BgCount - SigCount + OverlapCount << endl;
    cout << "hitsum_sig High : " << SigSum << " Low : " << SigSum_low << endl;
    cout << "hitsum_before High : " << BgSum << " Low : " << BgSum_low << endl;
    cout << "hitsum_Overflow High : " << OverSum << " Low : " << OverSum_low << endl;
    cout << "hitsum_after High : " << EmSum   << " Low : " << EmSum_low << endl;
    cout << "hitsum_err High : " << EmSum - BgSum + OverSum - SigSum << " Low : " << EmSum_low - BgSum_low + OverSum_low - SigSum_low << endl << endl;
  }

  if (fQAFlag) {
    hMpcExDiff->Fill(EmSum - BgSum + OverSum - SigSum);
    hMpcExNHit->Fill(EmCount - BgCount - SigCount + OverlapCount);

    if (IEvt < 5) {
      Fun4AllServer *se = Fun4AllServer::instance();
      exo_Sig->SetAxisRange(0, 7,"Z");
      hEventDis[0][IEvt] = (TH2D*)exo_Sig->Project3D("yx");
      hEventDis[0][IEvt]->SetName(Form("hEventDis%d_%d", 0, IEvt));
      hEventDis[0][IEvt]->GetZaxis()->SetRangeUser(0, 2000);
      se->registerHisto(hEventDis[0][IEvt]);

      exo_Bg->SetAxisRange(0, 7, "Z");
      hEventDis[1][IEvt] = (TH2D*)exo_Bg->Project3D("yx");
      hEventDis[1][IEvt]->SetName(Form("hEventDis%d_%d", 1, IEvt));
      hEventDis[1][IEvt]->GetZaxis()->SetRangeUser(0, 2000);
      se->registerHisto(hEventDis[1][IEvt]);

      exo_Em->SetAxisRange(0, 7, "Z");
      hEventDis[2][IEvt] = (TH2D*)exo_Em->Project3D("yx");
      hEventDis[2][IEvt]->SetName(Form("hEventDis%d_%d", 2, IEvt));
      hEventDis[2][IEvt]->GetZaxis()->SetRangeUser(0, 2000);
      se->registerHisto(hEventDis[2][IEvt]);
    }
  }

  return EVENT_OK;
}



int mMpcExMyEmbed::EventCheck(PHCompositeNode* topNode)
{
  double zvtx_Bg = fPHGlobal_Bg->getBbcZVertex();
  double zvtx_Sig = fPHGlobal_Sig->getBbcZVertex();

  if (fDebug) cout << "Vertex Bg : " << zvtx_Bg << " Sig : " << zvtx_Sig << endl; 

  if (fQAFlag) hVertexDiff->Fill(zvtx_Bg - zvtx_Sig);

  if (fabs(zvtx_Bg - zvtx_Sig) < fVertexDiff) {
    return EVENT_OK;
  }

  // Preventing infinite loop
  if (fNErr > MAXERR) {
    cout << PHWHERE << "Unable to match vertex please check sync" << endl;
    exit(-1);
  }

  fNErr++;
  cout << PHWHERE << "Vertex are not matching skipping event..." << endl;
  return ABORTEVENT;
}



int mMpcExMyEmbed::GetNode(PHCompositeNode* topNode)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  PHCompositeNode *SigNode = se->topNode(fSigNodeName.c_str());
  PHCompositeNode *BgNode = se->topNode(fBgNodeName.c_str());

  if (!SigNode)  {
    std::cout << PHWHERE << "Can not find Sim Node" << std::endl;
    return ABORTEVENT;
  }

  if (!BgNode)  {
    std::cout << PHWHERE << "Can not find Bg Node" << std::endl;
    return ABORTEVENT;
  }

  fMpcExRawHit_Bg = getClass<MpcExRawHit>(BgNode, "MpcExRawHit");
  if(!fMpcExRawHit_Bg) {
    std::cout << PHWHERE << "Can not find Back ground MpcExRawHit node No sense continuing" << std::endl;
    return ABORTEVENT;
  }


  fPHGlobal_Bg = getClass<PHGlobal>(BgNode, "PHGlobal");
  if(!fPHGlobal_Bg) {
    std::cout << PHWHERE << "Can not find Back ground PHGlobal" << std::endl;
    return ABORTEVENT;
  }

  fPHGlobal_Sig = getClass<PHGlobal>(SigNode, "PHGlobal");
  if(!fPHGlobal_Sig) {
    std::cout << PHWHERE << "Can not find Signal PHGlobal" << std::endl;
    return ABORTEVENT;
  }

  fMpcExCalibCont = getClass<TMpcExCalibContainer>(BgNode, "TMpcExCalibContainer");
  if(!fMpcExCalibCont) {
    std::cout << PHWHERE << "Can not find Signal TMpcExCalibContainer node No sense continuing" << std::endl;
    return ABORTEVENT;
  }

  fMpcExGeaHitCont = getClass<TMpcExGeaHitContainer>(SigNode, "TMpcExGeaHitContainer");
  if(!fMpcExGeaHitCont) {
    std::cout << PHWHERE << "Can not find Signal TMpcExGeaHitContainer node No sense continuing" << std::endl;
    return ABORTEVENT;
  }
  return EVENT_OK;
}



int mMpcExMyEmbed::FindHit(unsigned int key, std::vector<unsigned int> &raw)
{
  if (raw.size() <= 0) return -1;
  std::vector<unsigned int>::iterator raw_iter;
  raw_iter = std::lower_bound(raw.begin(), raw.end(), key << 16);
  int idx = raw_iter - raw.begin();
  if (((*raw_iter) >> 16) != key) idx = -1;
  //  cout << "idx : " << idx << " and " << (*raw_iter >> 16) << " " << key << endl;
  return idx;
}



int mMpcExMyEmbed::ZeroSup(int ADC, int Type, int arm, double ped, double ped_sig)
{
  double sigma_cut = -999;
  if (Type == 0 && arm == 0) sigma_cut = MpcExConstants::SOUTH_DCM_HIGH_SIGMA_CUT;
  if (Type == 1 && arm == 0) sigma_cut = MpcExConstants::SOUTH_DCM_LOW_SIGMA_CUT;
  if (Type == 0 && arm == 1) sigma_cut = MpcExConstants::NORTH_DCM_HIGH_SIGMA_CUT;
  if (Type == 1 && arm == 1) sigma_cut = MpcExConstants::NORTH_DCM_LOW_SIGMA_CUT;
  if (sigma_cut < -100) {
    cout << PHWHERE <<"Warning : Please check Type and Arm" << endl;
    return 0;
  }

  unsigned short threshold = ped + sigma_cut*ped_sig;
  if (ADC < threshold) return 0;

  return 1;
}
