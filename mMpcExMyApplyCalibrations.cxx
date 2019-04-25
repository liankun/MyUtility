#include "mMpcExMyApplyCalibrations.h"
#include "PHIODataNode.h"
#include "PHNodeIterator.h"
#include "PHCompositeNode.h"
#include "Fun4AllReturnCodes.h"
#include "recoConsts.h"
#include "getClass.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExCalib.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "TMpcExHitSet.h"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include "Exogram.h"
#include "MpcExRawHit.h"
#include "MpcExEventHeader.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TH2D.h"
#include "TH1D.h"
#include <functional>
#include <sstream>

mMpcExMyApplyCalibrations::mMpcExMyApplyCalibrations() : SubsysReco("MMPCEXMYAPPLYCALIBRATIONS") {

 //initialize the flag to default fail
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  _FailCellIDCheck[a][p][c][s] = 1; //true
	  hit_list[a][p][c][s].clear(); // make sure list is clear 		 
	}
      }
    }
  }

  recoConsts *myrc = recoConsts::instance();

  doCMNsubtract = myrc->get_IntFlag("MPCEX_CMN_SUBTRACT",0x0); 
  if(doCMNsubtract)
    std::cout<<PHWHERE<<" Performing CMN subtraction!"<<std::endl;

  doSigmaCut = myrc->get_IntFlag("MPCEX_SIGMA_CUT",0x1); 
  if(doSigmaCut)
    std::cout<<PHWHERE<<" Performing sigma cut on pedestal subtracted ADCs!"<<std::endl;

  doSigmaCutMIP = myrc->get_IntFlag("MPCEX_SIGMA_CUT_MIP",0x0); 
  if(doSigmaCutMIP){
    std::cout<<PHWHERE<<" Performing MIP sigma cut on pedestal subtracted ADCs!"<<std::endl;
    doSigmaCut = 0; // MIP cut overrides
  }

  calibMode = (mMpcExMyApplyCalibrations::Mode) myrc->get_IntFlag("MPCEXCALIBMODE",mMpcExMyApplyCalibrations::COMPLETE); 
  std::cout<<PHWHERE<<" Calibration Mode calibMode = ";
  switch(calibMode){
  case mMpcExMyApplyCalibrations::PEDESTAL_SUBTRACTED_ONLY:
    std::cout << "PEDESTAL_SUBTRACTED_ONLY" << std::endl;
    break; 
  case mMpcExMyApplyCalibrations::PEDESTAL_AND_CMN_SUBTRACTED_ONLY:
    std::cout << "PEDESTAL_AND_CMN_SUBTRACTED_ONLY" << std::endl;
    break; 
  case mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT:
    std::cout << "COMPLETE_FIXED_MC_PERFECT" << std::endl;
    break; 
  case mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC:
    std::cout << "COMPLETE_FIXED_MC" << std::endl;
    break; 
  case mMpcExMyApplyCalibrations::COMPLETE_FIXED_REALPED:
    std::cout << "COMPLETE_FIXED_REALPED" << std::endl;
    break; 
  case mMpcExMyApplyCalibrations::COMPLETE:
    std::cout << "COMPLETE" << std::endl;
    break; 
  default: 
    std::cout << "UNKNOWN (update the code!)" << std::endl;
  }

  eliminateBad = myrc->get_IntFlag("MPCEXKILLBADHITS",0x0); 

  applyStackCut = myrc->get_IntFlag("MPCEXCALIBAPPLYSTACK",0x0); 

  H_L_ConsistencyCut = myrc->get_IntFlag("MPCEX_HL_CONSISTENCY_CUT",0x0);

  disable_MPV_layer_adjust = myrc->get_IntFlag("MPCEX_NO_LAYER_MPV_ADJUST",0x0); 

  //set up the random number generator
  // 0 starting seed will generate unique seed
  // so long as each new instance of this module
  // is more than 1 second apart
  r3 = new TRandom3(0); 

  // Initialize the histogram pointers
  // If histograms are requiested this will be set up in InitRun()

  _outputfile = NULL;
  const unsigned int NMINIPADS = MpcExConstants::NMINIPADS_PER_PACKET;
  for(unsigned int i=0; i<NMINIPADS; i++) _histo_pedsub_highlow[i] = NULL;
  _histo_raw_low = NULL;
  _histo_raw_high = NULL;
  _histo_pedsub_low = NULL;
  _histo_pedsub_high = NULL;
  _histo_sensorCalib_low = NULL;
  _histo_sensorCalib_high = NULL;
  _histo_fullCalib_low = NULL;
  _histo_fullCalib_high = NULL;
  _histo_badcellid = NULL;
  _histo_low = NULL;
  _histo_high = NULL;
  _histo_combined = NULL;
  _histo_numHits_S = NULL;
  _histo_numHits_N = NULL;
  _histo_low_hotdead[0] = NULL;
  _histo_low_hotdead[1] = NULL;
  _histo_high_hotdead[0] = NULL;
  _histo_high_hotdead[1] = NULL;
  _histo_raw_low_parst = NULL;
  _histo_raw_high_parst = NULL;
  _histo_pedsub_low_parst = NULL;
  _histo_pedsub_high_parst = NULL;
  _histo_ped_low = NULL;
  _histo_ped_high = NULL;
  _histo_pedsigma_low = NULL;
  _histo_pedsigma_high = NULL;
  _histo_pedchi2_low = NULL;
  _histo_pedchi2_high = NULL;
  _histo_pedsub_low_statephase = NULL;
  _histo_pedsub_high_statephase = NULL;

}

mMpcExMyApplyCalibrations::~mMpcExMyApplyCalibrations(){
  if(_makeHisto) {
    delete _outputfile;
  }

  delete r3; 
}

int mMpcExMyApplyCalibrations::Init(PHCompositeNode *topNode)
{

  return EVENT_OK;

}

int mMpcExMyApplyCalibrations::InitRun(PHCompositeNode *topNode)
{

  recoConsts *myrc = recoConsts::instance();

  std::string histofilename = myrc->get_CharFlag("MpcExApplyCalibHistoFileName","");
  _makeHisto = histofilename != "";
  if(_makeHisto){
    _histo_arm = myrc->get_IntFlag("MpcExApplyCalibHistoArm",-1);
    _histo_packet = myrc->get_IntFlag("MpcExApplyCalibHistoPacket",-1);
    if(_histo_arm < 0 || _histo_arm>MpcExConstants::NARMS ||
       _histo_packet < 0 || _histo_packet>MpcExConstants::NPACKETS_PER_ARM
       ) {
      std::cout<<"To make histograms you need to set"<<std::endl;
      std::cout<<"\tMpcExApplyCalibHistoFileName <str>"<<std::endl;
      std::cout<<"\tMpcExApplyCalibHistoArm [0-1]"<<std::endl;
      std::cout<<"\tMpcExApplyCalibHistoPacket [0-7]"<<std::endl;
      std::cout<<"One or both of the last two is not specified -- NO HISTOS WILL BE PRODUCED"<<std::endl;
      _makeHisto = false;
    }

    _outputfile = new TFile(histofilename.c_str(),"RECREATE");

    const unsigned int NMINIPADS = MpcExConstants::NMINIPADS_PER_PACKET; // You freaking redefined NMINIPADS! Really? It isn't a redefinition. I renamed it intentionally. It is much easier to read this way and easier to copy and paste the ~30 times that I needed to. 
    for(unsigned int chipmap=0; chipmap<NMINIPADS; chipmap++){
      std::ostringstream hname;
      hname<<"pedsub_adc_highlow_"<<chipmap;
      _histo_pedsub_highlow[chipmap] = new TH2D(hname.str().c_str(),"Pedestal Subtracted High/Low ADC",256,-0.5,255.5,256,-0.5,255.5);
    }

    _histo_raw_low = new TH2D("raw_adc_low_bykey","Raw Low Gain ADC by chipmap",NMINIPADS+1,-1.5,NMINIPADS-0.5,256,-0.5,255.5);
    _histo_raw_high = new TH2D("raw_adc_high_bykey","Raw High Gain ADC by chipmap",NMINIPADS+1,-1.5,NMINIPADS-0.5,256,-0.5,255.5);
    _histo_pedsub_low = new TH2D("pedsub_adc_low_bykey","Pedestal-Subtracted Low Gain ADC by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);
    _histo_pedsub_high = new TH2D("pedsub_adc_high_bykey","Pedestal-Subtracted High Gain ADC by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);
    _histo_sensorCalib_low = new TH2D("sensorCalib_low_bykey","Sensor MIP Calibrated Low Gain by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.0,12.0);
    _histo_sensorCalib_high = new TH2D("sensorCalib_high_bykey","Sensor MIP Calibrated High Gain by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.0,12.0);
    _histo_fullCalib_low = new TH2D("fullCalib_low_bykey","Full MIP Calibrated Low Gain by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.0,12.0);
    _histo_fullCalib_high = new TH2D("fullCalib_high_bykey","Full MIP Calibrated High Gain by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.0,12.0);
    _histo_low_hotdead[0] = new Exogram("hotdeadmap_low_south","South hot/dead low-gain minipads",640,-19.5,19.5,640,-19.5,19.5,8,-0.5,7.5);
    _histo_low_hotdead[1] = new Exogram("hotdeadmap_low_north","North hot/dead low-gain minipads",640,-19.5,19.5,640,-19.5,19.5,8,-0.5,7.5);
    _histo_high_hotdead[0] = new Exogram("hotdeadmap_high_south","South hot/dead high-gain minipads",640,-19.5,19.5,640,-19.5,19.5,8,-0.5,7.5);
    _histo_high_hotdead[1] = new Exogram("hotdeadmap_high_north","South hot/dead high-gain minipads",640,-19.5,19.5,640,-19.5,19.5,8,-0.5,7.5);
    _histo_badcellid = new TH1D("badcellid","frequency of hits in bad-cell id sensors by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5);
    _histo_low = new TH2D("calib_elow_bykey","low gain energy deposit (keV) by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.,10000.0); 
    _histo_high = new TH2D("calib_ehigh_bykey","high gain energy deposit (keV) by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.,2000.0); 
    _histo_combined = new TH2D("calib_ecombined_bykey","combined energy deposit (keV) by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,256,0.,10000.0); 
    _histo_numHits_S = new TH1D("num_hits_S","Number of calibrated minipad hits (south)",NMINIPADS,-0.5,NMINIPADS-0.5); 
    _histo_numHits_N = new TH1D("num_hits_N","Number of calibrated minipad hits (north)",NMINIPADS,-0.5,NMINIPADS-0.5); 

    _histo_raw_low_parst = new TH2D("raw_adc_low_byparst","Raw Low Gain ADC by PARST",120,-0.5,120-0.5,257,-1.5,255.5);
    _histo_raw_high_parst = new TH2D("raw_adc_high_byparst","Raw High Gain ADC by PARST",120,-0.5,120-0.5,257,-1.5,255.5);

    _histo_pedsub_low_parst = new TH2D("pedsub_adc_low_parst","Pedestal-Subtracted Low Gain ADC by PARST",120,-0.5,120-0.5,291,-35.5,255.5);
    _histo_pedsub_high_parst = new TH2D("pedsub_adc_high_parst","Pedestal-Subtracted High Gain ADC by PARST",120,-0.5,120-0.5,291,-35.5,255.5);

    _histo_ped_low = new TH2D("ped_low_bykey","Pedestal by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);
    _histo_ped_high = new TH2D("ped_high_bykey","Pedestal by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);

    _histo_pedsigma_low = new TH2D("pedsigma_low_bykey","Pedestal Sigma by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,10,-0.5,9.5);
    _histo_pedsigma_high = new TH2D("pedsigma_high_bykey","Pedestal Sigma by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,10,-0.5,9.5);

    _histo_pedchi2_low = new TH2D("pedchi2_low_bykey","Pedestal Sigma by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);
    _histo_pedchi2_high = new TH2D("pedchi2_high_bykey","Pedestal Sigma by chipmap",NMINIPADS,-0.5,NMINIPADS-0.5,291,-35.5,255.5);

    _histo_pedsub_low_statephase = new TH2D("pedsub_adc_low_statephase","Pedestal-Subtracted Low Gain ADC by StatePhase",513,-0.5,512.5,291,-35.5,255.5);
    _histo_pedsub_high_statephase = new TH2D("pedsub_adc_high_statephase","Pedestal-Subtracted High Gain ADC by StatePhase",513,-0.5,512.5,291,-35.5,255.5);

  } 

  return EVENT_OK;

}

int mMpcExMyApplyCalibrations::process_event(PHCompositeNode *topNode){

  MpcExRawHit *raw_hits = findNode::getClass<MpcExRawHit>(topNode,"MpcExRawHit");
  if(raw_hits == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find MpcExRawHit"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you are reading the MPC-EX DSTs."<<std::endl;
    return ABORTRUN;
  }

  MpcExEventHeader *evt_head = findNode::getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");
  if(evt_head == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find MpcExEventHeader"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you are reading the MPC-EX DSTs."<<std::endl; //is this right?
    return ABORTRUN;
  }

  TMpcExCalibContainer *calibs = findNode::getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(calibs == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find TMpcExCalibContainer"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you load mMpcExCreateNodeTree."<<std::endl;
    return ABORTRUN;
  }

  TMpcExHitContainer *hits = findNode::getClass<TMpcExHitContainer>(topNode,"TMpcExHitContainer");
  if(hits == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find TMpcExHitContainer"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you load mMpcExCreateNodeTree."<<std::endl;
    return ABORTRUN;
  }

  // Make the stack cut for single event buffering (if requested)
  if( applyStackCut && (evt_head->getStack() != 1) ) return ABORTEVENT;

  // Check the StatePhase for all the packets if stack>1
  if(evt_head->getStack()>1) {
    if(!StatePhaseCheck(evt_head)) return ABORTEVENT;
  }

  // Check the PARST times for all the packets
  int PARSTCode = PARSTCheck(evt_head); 
  if(!PARSTCode) return ABORTEVENT; 

  //find which chips give inconsistent cellID's compared to rest in the chain
  CellIDCheck(evt_head);

  unsigned int nraw = raw_hits->getnhits();
  for(unsigned int iraw=0; iraw<nraw; iraw++){
    //we clone the hit from TMpcExHitSet
    //because *itr is the pointer owned by TMpcExHitSet
    //and this pointer will eventually be owned by 
    //the TMpcExHitContainer on the node tree
    TMpcExHit *hit = hits->getUncalHit(raw_hits->getOnlineKey(iraw));
    hit->set_low(raw_hits->getladc(iraw));
    hit->set_high(raw_hits->gethadc(iraw));
    hit->set_state_low(TMpcExHit::ADC);
    hit->set_state_high(TMpcExHit::ADC);

    //grab the calibration by the key
    unsigned int key = hit->key();

    //get calibration object
    TMpcExCalib *calib = calibs->get(key);
    if(calib == NULL){
      std::cout<<PHWHERE<<" Something is terribly wrong -- I could not get the calibrations for minipad with key: "
	       <<hit->key()<<std::endl;
      std::cout<<PHWHERE<<" Doing nothing..."<<std::endl;
      continue;
    }

    //check if chip's cellID is okay
    unsigned short arm = hit->arm();
    unsigned short packet = hit->packet();
    unsigned short chain = hit->chain();
    unsigned short chip = hit->chip();
    unsigned short chipmap = hit->chipmap();
      
    // PARST check 
    if( ((PARSTCode==2) && (arm==0)) || ((PARSTCode==3) && (arm==1)) ){
      continue; 
    }

    hit->set_state_low(TMpcExHit::ADC);
    hit->set_state_high(TMpcExHit::ADC);

    if (_FailCellIDCheck[arm][packet][chain][chip])
      {
	if(_makeHisto){
	  _histo_badcellid->Fill(chipmap);
	}
	continue;
      }

    // Mark dead/hot channels first
    if(_makeHisto){
      if(calib->low_dead_hot_status()>0){
	_histo_low_hotdead[arm]->FillEx(chipmap,1);
      }
      if(calib->high_dead_hot_status()>0){
	_histo_high_hotdead[arm]->FillEx(chipmap,1);
      }
    }

    // Flag channels that weren't in the PRDF - 
    // the packet code just returns 0, meaning this came
    // along for the ride 

    if(hit->low()<=0.0){
      hit->set_status_low(TMpcExHit::ADC_ZERO_SUPPRESSED);
    }

    if(hit->high()<=0.0){
      hit->set_status_high(TMpcExHit::ADC_ZERO_SUPPRESSED);
    }

    // Flag hot/dead channels
    // (Unless perfect MC requested)

    if(calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT){
      if(calib->low_dead_hot_status()>0){
	if(calib->low_dead_hot_status()==2)
	  hit->set_status_low(TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY);
	else
	  hit->set_status_low(TMpcExHit::DEAD_HOT);
      }

      if(calib->high_dead_hot_status()>0){
	hit->set_status_high(TMpcExHit::DEAD_HOT);
      }
    }
    
    //need to determine if we will fill histograms for this hit
    bool fillHistos = false;
    if(_makeHisto){
      unsigned short arm = hit->arm();
      unsigned short packet = hit->packet();
      fillHistos = arm==_histo_arm && packet==_histo_packet;
    }

    //fill the raw ADC distributions
    if(fillHistos && _makeHisto){
      _histo_ped_low->Fill(chipmap,calib->low_pedestal());
      _histo_ped_high->Fill(chipmap,calib->high_pedestal());
      _histo_pedsigma_low->Fill(chipmap,calib->low_pedestal_width());
      _histo_pedsigma_high->Fill(chipmap,calib->high_pedestal_width());
      _histo_pedchi2_low->Fill(chipmap,calib->low_pedestal_chi2());
      _histo_pedchi2_high->Fill(chipmap,calib->high_pedestal_chi2());
      _histo_raw_low->Fill(chipmap,hit->low());
      _histo_raw_high->Fill(chipmap,hit->high());
      _histo_raw_low_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->low());
      _histo_raw_high_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->high());
    }

    //subtract off the pedestal...
    //but only for healthy channels
      
    bool bad_low_pedestal = false;
    bool bad_high_pedestal = false;
    float low_pedestal; 
    float high_pedestal; 

    if( (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) && 
	(calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC) ){
      bad_low_pedestal = (calib->low_pedestal_chi2()<0) || 
	((hit->status_low()&TMpcExHit::ADC_ZERO_SUPPRESSED)!=0) ||
	((hit->status_low()&TMpcExHit::DEAD_HOT)!=0);
      bad_high_pedestal = (calib->high_pedestal_chi2()<0) || 
	((hit->status_high()&TMpcExHit::ADC_ZERO_SUPPRESSED)!=0) || 
	((hit->status_high()&TMpcExHit::DEAD_HOT)!=0); 
      low_pedestal = calib->low_pedestal(); 
      high_pedestal = calib->high_pedestal(); 
    }
    else{
      bad_low_pedestal = (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) && 
	((hit->status_low()&TMpcExHit::DEAD_HOT)!=0);
      bad_high_pedestal = (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) && 
	((hit->status_high()&TMpcExHit::DEAD_HOT)!=0); 
      low_pedestal = MpcExConstants::FIXED_LOW_PEDESTAL; 
      high_pedestal = MpcExConstants::FIXED_HIGH_PEDESTAL; 
    }
      
    if(!bad_low_pedestal){
      hit->set_low(hit->low()-low_pedestal);
      hit->set_state_low(TMpcExHit::PEDESTAL_SUBTRACTED); 	     
    }
    else{
      hit->set_status_low(TMpcExHit::BAD_PEDESTAL_CALIBRATION); 	    
    }

    if(!bad_high_pedestal){
      hit->set_high(hit->high()-high_pedestal);
      hit->set_state_high(TMpcExHit::PEDESTAL_SUBTRACTED); 	     
    }
    else{
      hit->set_status_high(TMpcExHit::BAD_PEDESTAL_CALIBRATION); 	    
    }
    
    // At this point, remaining "good" hits will have been marked with a 
    // state of PEDESTAL_SUBTRACTED and status === 0 || LOW_RANGE_LARGE_ADC_ONLY

    // If CMN subtraction is NOT requested do the sigma cut here
    // This reduces the number of hits that need to be shoveled around. 
    // If CMN subtraction is requested then the sigma cut must be done *after*
    // CMN subtraction has been completed.

    if(doSigmaCut && !doCMNsubtract) SigmaCut(hit,calib);  
 
    // Pedestal subtracted ADC histograms
    if(fillHistos && _makeHisto){
      if( (hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED) && 
	  ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY)) ) {
	_histo_pedsub_low->Fill(chipmap,hit->low());
	_histo_pedsub_low_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->low());
	unsigned int statephase = evt_head->getStatephase(arm*MpcExConstants::NPACKETS_PER_ARM + packet) & 0xfff; 
	_histo_pedsub_low_statephase->Fill(statephase,hit->low());
      }
      if( (hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_high()==0) ) {
	_histo_pedsub_high->Fill(chipmap,hit->high());
	_histo_pedsub_high_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->high());
	unsigned int statephase = evt_head->getStatephase(arm*MpcExConstants::NPACKETS_PER_ARM + packet) & 0xfff; 
	_histo_pedsub_high_statephase->Fill(statephase,hit->high());
      }
      if( (hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED) && 
	  ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY)) && 
	  (hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_high()==0) ) {
	_histo_pedsub_highlow[chipmap]->Fill(hit->low(),hit->high());
      }
    }

    if(calibMode==mMpcExMyApplyCalibrations::PEDESTAL_SUBTRACTED_ONLY){

      // Store hits with EITHER a valid high or low gain ADC
      // All others get deleted.
      if( ((hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED) && 
	   ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY))) || 
	  ((hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_high()==0)) ){
        hits->addHit(hit);
      }

    }
    else{
      // Add the hit to the storage lists
      hit_list[arm][packet][chain][chip].push_back(hit);
    }

  }

  // Stop here if only pedestal subtraction was requested 
  if(calibMode==mMpcExMyApplyCalibrations::PEDESTAL_SUBTRACTED_ONLY) return EVENT_OK; 

  // Common Mode Noise Subtraction
  if(doCMNsubtract) ApplyCommonModeNoiseSubtraction();

  // Energy calibration - we now need to loop over the 
  // stored hits and continue with the calibrations
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip

	  std::vector<TMpcExHit*>::iterator iter;
	  for (iter = hit_list[a][p][c][s].begin(); iter != hit_list[a][p][c][s].end(); ){ 

	    TMpcExHit *hit = *iter;

	    //grab the calibration by the key
	    unsigned int key = hit->key();

	    //get calibration object
	    TMpcExCalib *calib = calibs->get(key);
	    if(calib == NULL){
	      std::cout<<PHWHERE<<" Something is terribly wrong -- I could not get the calibrations for minipad with key: "
		       <<hit->key()<<std::endl;
	      std::cout<<PHWHERE<<" Doing nothing..."<<std::endl;
	      ++iter; 
	      continue;
	    }

	    //need to determine if we will fill histograms for this hit
	    unsigned short chipmap = hit->chipmap();
	    unsigned short arm = hit->arm();
	    unsigned short packet = hit->packet();
	    bool fillHistos = false;
	    if(_makeHisto){
	      fillHistos = arm==_histo_arm && packet==_histo_packet;
	    }

	    // At this point, remaining "good" hits will be marked with a 
	    // state of PEDESTAL_SUBTRACTED -OR- PEDESTAL_AND_CMN_SUBTRACTED and status === 0 || LOW_RANGE_LARGE_ADC_ONLY

	    // The sigma cut must be applied here, after the CMN subtraction, 
	    // if CMN subtraction has been requested. 
	    // The CMN subtraction looks for the lowest value to subtract; what we really
	    // want is a channel close to pedestal, so we can't cut those out before we
	    // do CMN subtraction. 

	    if(doSigmaCut && doCMNsubtract) SigmaCut(hit,calib); 

	    // To go on to energy calibration we require:
	    // -> Both the low and high gain are valid -OR-
	    // -> The high gain is invalid and the low gain is valid over full range -OR-
	    // -> The high gain is valid and the low gain is only valid in the high range, or it is pedestal or
	    //    zero suppressed.
	    // All others hits get deleted
	    // Note that we do NOT allow the case where the high gain is good but the low gain is dead/hot. These
	    // minipads will just be thrown out. 
	    if( ((hit->status_low()==0)&&(hit->status_high()==0)) ||
		((hit->status_low()==0)&&(hit->status_high()!=0)) ||
		(((hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY) || 
		  (hit->status_low()==TMpcExHit::ADC_ZERO_SUPPRESSED) ||
		  (hit->status_low()==TMpcExHit::FAILS_SIGMA_CUT) ) && (hit->status_high()==0)) ){
	      // Nothing here, move along....
	    }
	    else{
	      ++iter; 
	      continue; 
	    }

	    // Pedestal subtracted ADC histograms
	    if(fillHistos && _makeHisto){
	      if( (hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED) && 
		  ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY)) ) {
		_histo_pedsub_low->Fill(chipmap,hit->low());
		_histo_pedsub_low_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->low());
		unsigned int statephase = evt_head->getStatephase(arm*MpcExConstants::NPACKETS_PER_ARM + packet) & 0xfff; 
		_histo_pedsub_low_statephase->Fill(statephase,hit->low());
	      }
	      if( (hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED) && (hit->status_high()==0) ) {
		_histo_pedsub_high->Fill(chipmap,hit->high());
		_histo_pedsub_high_parst->Fill(evt_head->getPARSTTime(arm*MpcExConstants::NPACKETS_PER_ARM + packet),hit->high());
		unsigned int statephase = evt_head->getStatephase(arm*MpcExConstants::NPACKETS_PER_ARM + packet) & 0xfff; 
		_histo_pedsub_high_statephase->Fill(statephase,hit->high());
	      }
	      if((hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED) && 
		 (hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED) && 
		 ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY)) && 
		 (hit->status_high()==0) ) {
		_histo_pedsub_highlow[chipmap]->Fill(hit->low(),hit->high());
	      }
	    }

	    // Stop here if only pedestal and CMN subtraction was requested 
	    if(calibMode==mMpcExMyApplyCalibrations::PEDESTAL_AND_CMN_SUBTRACTED_ONLY) {
	      hits->addHit(hit);
	      ++iter; 
	      continue; 
	    }
 
	    // Energy calibration
	      
	    float MIP_sensor = 0.0; 
	    float HL_ratio = 0.0; 
	    float MPV_layer_adjust = MpcExConstants::MIP_IN_keV; 

	    // We need to include a check in here if the calibrations are 
	    // valid, and dump out if they are not:
	    // THIS HAPPENS FOR FIXED CALIBRATIONS AS WELL
	      
	    bool MPs_OK = false; 
	    bool HL_OK = false; 

	    if(calib->get_mip_in_sensor()>0.0){
	      MIP_sensor = calib->get_mip_in_sensor();
	      MPs_OK = true; 
	    }

	    if(calib->get_high_low_ratio()>0.0){
	      // L/H ratio stored in DB
	      HL_ratio = 1.0/calib->get_high_low_ratio();
	      HL_OK = true; 
	    }

	    if( (calibMode == mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) || 
		(calibMode == mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC) ||
		(calibMode == mMpcExMyApplyCalibrations::COMPLETE_FIXED_REALPED) ){

	      // Reset the values for FIXED calibrations
		
	      MIP_sensor = MpcExConstants::FIXED_MIP_L27;
	      if((hit->layer()==0) || (hit->layer()==1)) MIP_sensor = MpcExConstants::FIXED_MIP_L01;
	      HL_ratio = MpcExConstants::FIXED_HL_RATIO; 

	      if( calibMode == mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT ) {
		MPs_OK = true; 
		HL_OK = true; 
	      }

	    }
	    else{		
	      // Check the consistency of the H/L ratio and the data, 
	      // 5-sigma cut (sigma is on L/H value) 
	      if(H_L_ConsistencyCut && HL_OK && 
		 (hit->status_low()==0) && (hit->status_high()==0) &&
		 (hit->high()>0.0) && (hit->low()>0.0) && 
		 (hit->high()<(MpcExConstants::MAX_ADC_COMBINE-calib->high_pedestal())) ){
		float fit_low = calib->get_high_low_offset() + calib->get_high_low_ratio()*hit->high();
		float lh_dist = fabs(fit_low-hit->low())/sqrt(pow(calib->get_high_low_ratio(),2)+1.0);
		if(lh_dist > 5.0*calib->get_high_low_sigma()) HL_OK = false;
	      }
	    }

	    // bail out if hit cannot be calibrated
	    if(!HL_OK || !MPs_OK){

	      if(!eliminateBad)
		hits->addHit(hit);

	      ++iter; 
	      continue; 

	    }
      
	    // For histogramming purposes, separate out application of sensor-by-sensor and minipad-by-minipad corrections. 
	    // For comparison with reference histograms the MIP peak is at 1, i.e. no energy scale is applied. 
	    // Also, no random shift is applied.

	    if(fillHistos && _makeHisto){

	      if((hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)) 
		_histo_sensorCalib_low->Fill(chipmap,hit->low()/MIP_sensor*HL_ratio);

	      if((hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)) 
		_histo_sensorCalib_high->Fill(chipmap,hit->high()/MIP_sensor);
		
	    }
    
	    // Get the minipad-by-minipad MIP calibration
	    // and the layer-by-layer MPV corrections

	    if( (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) && 
		(calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC) && 
		(calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_REALPED) ){

	      if(calib->get_minipad_mip_correction()>0.0) {
		MIP_sensor *= calib->get_minipad_mip_correction();
		MIP_sensor /= calib->get_mip_correction(); // Liankun's charged track MIP correction
		MIP_sensor /= calib->get_smear_scale(); // Liankun's final charged track correction
	      }
	      else{
		if(!eliminateBad)
		  hits->addHit(hit);

		++iter; 
		continue; 
	      }

	      if( (!disable_MPV_layer_adjust) && (calib->get_mip_layer_mpv()>0.0) ) 
		MPV_layer_adjust = calib->get_mip_layer_mpv(); 
		    
	    }

	    if(fillHistos && _makeHisto){

	      if((hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)) 
		_histo_fullCalib_low->Fill(chipmap,hit->low()/MIP_sensor*HL_ratio);

	      if((hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)) 
		_histo_fullCalib_high->Fill(chipmap,hit->high()/MIP_sensor);
	  
	    }

	    // Gain calibration for high and low gain channels

	    if(((hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED)||
		(hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED))
	       && ((hit->status_low()==0)||(hit->status_low()==TMpcExHit::LOW_RANGE_LARGE_ADC_ONLY))
	       && (hit->low()>0.0) ){
       
	      hit->set_low((hit->low()+r3->Rndm())*(MpcExConstants::MIP_IN_keV/MIP_sensor)*HL_ratio);

	      // Layer MIP adjustments
	      hit->set_low(hit->low()*(MpcExConstants::MIP_IN_keV/MPV_layer_adjust)); 

	      hit->set_state_low(TMpcExHit::GAIN_CALIBRATED);

	    }

	    double EMAX_HIGH = 0.0; 
	    if(((hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED)||
		(hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED))
	       && (hit->status_high()==0) && (hit->high()>0.0) ){

	      hit->set_high((hit->high()+r3->Rndm())*(MpcExConstants::MIP_IN_keV/MIP_sensor));

	      // Layer MIP adjutments from Liankun's offline MIP analysis
	      hit->set_high(hit->high()*(MpcExConstants::MIP_IN_keV/MPV_layer_adjust)); 

	      hit->set_state_high(TMpcExHit::GAIN_CALIBRATED);

	      float high_pedestal; 
	      if( (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC_PERFECT) && 
		  (calibMode != mMpcExMyApplyCalibrations::COMPLETE_FIXED_MC) )
		high_pedestal = calib->high_pedestal(); 
	      else
		high_pedestal = MpcExConstants::FIXED_HIGH_PEDESTAL; 
     
	      EMAX_HIGH = (MpcExConstants::MAX_ADC_COMBINE-high_pedestal)*(MpcExConstants::MIP_IN_keV/MIP_sensor);
	    }
	    
	    // The MIP sigm cut is in terms of energy, so do that here (before combining)

	    if(doSigmaCutMIP) SigmaCutMIP(hit,calib); 

	    // Generate the combined energy value

	    if( (hit->isGoodGCLowHit()|| hit->isGoodGCLowHitHighRange()) && 
		hit->isGoodGCHighHit() ){
	  
	      if(hit->high() < EMAX_HIGH)
		hit->set_combined(hit->high());
	      else
		hit->set_combined(hit->low());

	      hit->set_state_combined(TMpcExHit::VALID); 
	    
	    }
	    else if( hit->isGoodGCLowHit() )
	      {
		hit->set_combined(hit->low());	    
		// Is the high gain dead/hot? 
		if(calib->high_dead_hot_status()>0)
		  hit->set_state_combined(TMpcExHit::VALID_LOW_ONLY); 		
		else
		  hit->set_state_combined(TMpcExHit::VALID_LOW_EXPECTED_BOTH); 
	      }
	    else if( hit->isGoodGCHighHit() )
	      {
		hit->set_combined(hit->high());	    
		// We only get here if the low was zero suppressed or
		// eliminated by the sigma cut.
		hit->set_state_combined(TMpcExHit::VALID); 		  
	      }
	    else{
	      hit->set_combined(0.0);	    
	      hit->set_state_combined(TMpcExHit::INVALID); 
	    }

	    // combined hit histogram 
	    if(fillHistos && _makeHisto){
	      if(hit->state_low() == TMpcExHit::GAIN_CALIBRATED){
		_histo_low->Fill(chipmap,hit->low());
	      }
	      if(hit->state_high() == TMpcExHit::GAIN_CALIBRATED){
		_histo_high->Fill(chipmap,hit->high());
	      }
	      if(hit->isGoodCombinedHit())_histo_combined->Fill(chipmap,hit->combined()); 
	    }

	    // Delete the hit (and don't save it) if requested to 
	    // reject bad hits
	    if(!eliminateBad){
	      hits->addHit(hit);
	    }
	    else{
	      if(hit->isGoodCombinedHit())
		hits->addHit(hit);
	    }

	    ++iter; 

	  }
	  // Clear this list for the next event
	  hit_list[a][p][c][s].clear(); 
	}
      }
    }
  }

  if(_makeHisto){
    _histo_numHits_S->Fill(hits->sHits()); 
    _histo_numHits_N->Fill(hits->nHits()); 
  }

  return EVENT_OK;
}

int mMpcExMyApplyCalibrations::CellIDCheck(MpcExEventHeader *evt_head){
  //this method stores a list of svx4s that fail the cellid check in this event

  //fill map and array of cellID values for each svx4
  unsigned int Svx4CellIDs[2][8][48] = {{{0}}};
  for (unsigned int i = 0; i < evt_head->getCellIDsSize(); i++){
    //read event header to get cell ID for each chip
    unsigned int a = evt_head->getCellIDsArm(i);
    unsigned int p = evt_head->getCellIDsPkt(i);
    unsigned int s = evt_head->getCellIDsSVXID(i); //this is the chip + 12*chain
    unsigned int cellIDvalue =  evt_head->getCellIDsValue(i);
    Svx4CellIDs[a][p][s] = cellIDvalue;
  }

  //loop over quadrants
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 

	//make and clear cellIDMap
	std::map<unsigned int, int> CellIDMap;
	CellIDMap.clear();

	//count frequency of each cellID value
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  //initialize the flag to default fail
	  _FailCellIDCheck[a][p][c][s] = 1; //true
	  
	  //find cellID value for a given arm, packet, chain, chip
	  int svnum = c*12+s;
	  unsigned int cellID = Svx4CellIDs[a][p][svnum];
	  
	  //if it is new to the map, fill it in
	  if (CellIDMap.find(cellID)==CellIDMap.end()) 
	    { 
	      CellIDMap[cellID] = 1;   
	    }
	  else 	    //otherwise increment cellID count
	    CellIDMap[cellID]++;
	}
	
	//find most common non0 cellID
	int maxFreq = -1;
	unsigned int MostCommonCellID = 9999;
	std::map<unsigned int, int>::iterator it;
	for (it = CellIDMap.begin(); it != CellIDMap.end(); it++){
	  unsigned int cellID = it->first;
	  int Freq = it->second;
	  if (Freq > maxFreq && cellID!=0)
	    {
	      maxFreq = Freq;
	      MostCommonCellID = cellID;
	    }
	}	      
	//if all cellIDs=0, then MostCommonCellID=9999 and all in this quadrant fail the check

	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  int svnum = c*12+s;
	  unsigned int cellID = Svx4CellIDs[a][p][svnum];
	  if (cellID == MostCommonCellID) //if most common it doesn't fail
	    {
	      _FailCellIDCheck[a][p][c][s] = 0; //false
	    }
	}
      }
    }
  }

  return 0;
}

int mMpcExMyApplyCalibrations::End(PHCompositeNode *topNode)
{

  if(_makeHisto){
    _outputfile->Write();
    _outputfile->Close();
  }
  return 0;
}

void mMpcExMyApplyCalibrations::ApplyCommonModeNoiseSubtraction(){

  // Apply common mode noise subtraction by determining the lowest 
  // ADC in each SVX4 individually for the high and low gain channels, 
  // and subtracting that ADC value from all other channels. 

  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //SVX4

	  // Determine the smallest high gain, low gain CMN value with a given range
	  // from the pedestal subtracted ADC data

	  float low_gain_min_ADC = 300.0;
	  float high_gain_min_ADC = 300.0;

	  std::vector<TMpcExHit*>::iterator iter;
	  for (iter = hit_list[a][p][c][s].begin(); iter != hit_list[a][p][c][s].end(); ){ 

	    TMpcExHit *hit = *iter;

	    if( (hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_low()==0) )
	      if( hit->low()<low_gain_min_ADC ) low_gain_min_ADC = hit->low(); 

	    if( (hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_high()==0) )
	      if( hit->high()<high_gain_min_ADC ) high_gain_min_ADC = hit->high();

	    ++iter; 

	  }

	  // Subtract the CMN values from the all the ADC's for this chip
	  
	  for (iter = hit_list[a][p][c][s].begin(); iter != hit_list[a][p][c][s].end(); ){ 

	    TMpcExHit *hit = *iter;

	    if( (hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_low()==0) ){
	      hit->set_low(hit->low()-low_gain_min_ADC); 
	      hit->set_state_low(TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED); 	     
	    }  

	    if( (hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED) && (hit->status_high()==0) ){
	      hit->set_high(hit->high()-high_gain_min_ADC); 
	      hit->set_state_high(TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED); 	     
	    }  

	    ++iter; 

	  }

	}
      }
    }
  }

  return; 
}

// Check the PARST times - better be the same for all the packets in an 
// arm, and not within the bad range for each arm. 
int mMpcExMyApplyCalibrations::PARSTCheck(MpcExEventHeader *evt_head){

  // Run-16 J. Lajoie 5/28/2016
  int nbadp[MpcExConstants::NARMS] = {6,6};
  int bad_parsttime[MpcExConstants::NARMS][6] = { {32,33,43,44,45,46},{32,33,43,44,45,46} }; 
  int retval = 1; // 1=both arms OK, 2=skip arm=0, 3=skip arm=1
  
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop

    int prev_parsttime = -1; 
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet

      int parsttime = (int) evt_head->getPARSTTime(a*MpcExConstants::NPACKETS_PER_ARM+p); 
      if((parsttime!=prev_parsttime) && (prev_parsttime>0)){
	if(a==0) {
	  retval = 2;
	}
	else if (a==1){
	  if(retval==2) 
	    return 0; // skip event, both arms bad
	  else
	    retval = 3; 
	}	
      }
      else{
	prev_parsttime = parsttime;
	for(int i=0; i<nbadp[a]; i++){
	  if(parsttime==bad_parsttime[a][i]){
	    if(a==0) {
	      retval = 2;
	    }
	    else if (a==1){
	      if(retval==2) 
		return 0; // skip event, both arms bad
	      else
		retval = 3; 
	    }
	  }
	}
      }

    }

  }

  return retval; 
}

int mMpcExMyApplyCalibrations::StatePhaseCheck(MpcExEventHeader *evt_head){


  // Run-16 J. Lajoie 3/30/2016
  int nbadsp[MpcExConstants::NARMS] = {51,51};
  unsigned bad_statephase[MpcExConstants::NARMS][51] = { {39,40,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,84,85,86,118,119,150,151,182,183,184,201,213,214,215,216,248,249,250,281,282,283,312,313,346,347,378,388,411,443,475,476,499}, {39,40,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,84,85,86,118,119,150,151,182,183,184,201,213,214,215,216,248,249,250,281,282,283,312,313,346,347,378,388,411,443,475,476,499} }; 

  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for(int i=0; i<nbadsp[a]; i++){
        unsigned int statephase = evt_head->getStatephase(a*MpcExConstants::NPACKETS_PER_ARM + p) & 0xfff; 
        if(statephase==bad_statephase[a][i]) return 0;
      }
    }
  }
  
  return 1; 

}

void mMpcExMyApplyCalibrations::SigmaCut(TMpcExHit *hit, TMpcExCalib *calib) 
{

  float low_sigma_cut;
  float high_sigma_cut; 
  float low_pedestal_width; 
  float high_pedestal_width; 

  if( (calibMode==mMpcExMyApplyCalibrations::PEDESTAL_SUBTRACTED_ONLY) || 
      (calibMode==mMpcExMyApplyCalibrations::PEDESTAL_AND_CMN_SUBTRACTED_ONLY)){

    if(hit->arm()==0) {
      low_sigma_cut = MpcExConstants::SOUTH_DCM_LOW_SIGMA_CUT; 
      high_sigma_cut = MpcExConstants::SOUTH_DCM_HIGH_SIGMA_CUT; 
    }
    else{
      low_sigma_cut = MpcExConstants::NORTH_DCM_LOW_SIGMA_CUT;
      high_sigma_cut = MpcExConstants::NORTH_DCM_HIGH_SIGMA_CUT; 
    }

    low_pedestal_width = calib->low_pedestal_width(); 
    high_pedestal_width = calib->high_pedestal_width(); 

  }
  else{

    low_sigma_cut =  MpcExConstants::CAL_L27_LOW_SIGMA_CUT; 
    if((hit->layer()==0) || (hit->layer()==1)) low_sigma_cut = MpcExConstants::CAL_L01_LOW_SIGMA_CUT; 

    high_sigma_cut = MpcExConstants::CAL_L27_HIGH_SIGMA_CUT;
    if((hit->layer()==0) || (hit->layer()==1)) high_sigma_cut = MpcExConstants::CAL_L01_HIGH_SIGMA_CUT; 

    if(calibMode==mMpcExMyApplyCalibrations::COMPLETE){
      low_pedestal_width = calib->low_pedestal_width(); 
      high_pedestal_width = calib->high_pedestal_width(); 
    }
    else{
      low_pedestal_width = MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH; 
      high_pedestal_width = MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH; 
    }

  }
		
  if(hit->low()<=(low_sigma_cut*low_pedestal_width) )
    hit->set_status_low(TMpcExHit::FAILS_SIGMA_CUT );

  if (hit->high()<=(high_sigma_cut*high_pedestal_width) ) 
    hit->set_status_high(TMpcExHit::FAILS_SIGMA_CUT );

}

void mMpcExMyApplyCalibrations::SigmaCutMIP(TMpcExHit *hit, TMpcExCalib *calib) 
{

  double cor=0;

  if(hit->low()<=calib->get_mip_corr_cutoff_position()*cor || hit->low()<50)
    hit->set_status_low(TMpcExHit::FAILS_SIGMA_CUT );

  if (hit->high()<=calib->get_mip_corr_cutoff_position()*cor || hit->high()<50) 
    hit->set_status_high(TMpcExHit::FAILS_SIGMA_CUT );

}
