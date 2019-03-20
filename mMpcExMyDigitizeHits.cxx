#include "mMpcExMyDigitizeHits.h"
#include "Fun4AllServer.h"
#include "Fun4AllReturnCodes.h"
#include "recoConsts.h"
#include "PHCompositeNode.h"
#include "getClass.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExCalib.h"
#include "TMpcExGeaHit.h"
#include "TMpcExGeaHitContainer.h"
#include "TMpcExHit.h"
#include "TMpcExHitSet.h"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include "MpcExPISAEventHeader.h"
#include <TRandom3.h>
#include <cmath>
#include <iostream>
#include <map>
#include <fstream>
#include <MpcExMapper.h>

#include "TFile.h"
#include "TH2.h"


mMpcExMyDigitizeHits::mMpcExMyDigitizeHits() : SubsysReco("MMPCEXMYDIGITIZEHITS") {
  _dice = new TRandom3(0);
  _energy_smear_width = 0.000001;
  _sup_mod = FIX_SUP;
  _smear_mod = SAME_SMEAR;

  recoConsts *myrc = recoConsts::instance();
  makeHisto = myrc->get_IntFlag("MpcExDigitizeHitsHisto",0x0); 
  outputfile = NULL;
  _histo_low = NULL;
  _histo_high = NULL;
  _histo_low_dcm = NULL;
  _histo_high_dcm = NULL;

  for(unsigned int i=0;i<MpcExConstants::NMINIPADS;i++){
    _default_cut_high_adc[i] = -9999;
    _fit_mpv_cut_high_adc[i] = -9999;
    _default_cut_low_adc[i] = -9999;
    _fit_mpv_cut_low_adc[i] = -9999;
    _e_smear[i] = 0;
  }

  //set _e_smear value
  int key=0;
  float tmp_value[7];
  std::ifstream in_txt("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MyUtility/Data_File/fit_match_landau_all_scale_smear_1sigma_start_based_on_old_cor_scale_up_1_1.txt");
  if(in_txt.is_open()){
    while(in_txt>>key>>tmp_value[0]>>tmp_value[1]>>tmp_value[2]
                     >>tmp_value[3]>>tmp_value[4]>>tmp_value[5]
		     >>tmp_value[6]
	 ){ 
      if(tmp_value[3]>=0.1) _e_smear[key] = tmp_value[5];
    }
  }
  else{ 
    std::cout<<"open smear file failed !"<<std::endl;
  }
}


mMpcExMyDigitizeHits::~mMpcExMyDigitizeHits(){
  delete _dice;
  if(makeHisto)
    delete outputfile;
}

int mMpcExMyDigitizeHits::Init(PHCompositeNode *topNode)
{

  if(makeHisto){
    outputfile = new TFile("MpcExDigitizeHitsHisto.root","RECREATE");
    _histo_low = new TH2D("low_adc_bykey","Low gain ADC by key",49152,-0.5,49151.5,256,0.,256.0); 
    _histo_high = new TH2D("high_adc_bykey","High gain ADC by key",49152,-0.5,49151.5,256,0.,256.0); 
    _histo_low_dcm = new TH2D("low_adc_bykey_dcm","Low gain ADC by key (DCM cut)",49152,-0.5,49151.5,256,0.,256.0); 
    _histo_high_dcm = new TH2D("high_adc_bykey_dcm","High gain ADC by key (DCM cut)",49152,-0.5,49151.5,256,0.,256.0); 
  }

  return 0; 

}

int mMpcExMyDigitizeHits::process_event(PHCompositeNode *topNode){

  //These are the "Gea Hits" at first that will get turned into digitized hits
  TMpcExGeaHitContainer *geahits = findNode::getClass<TMpcExGeaHitContainer>(topNode, "TMpcExGeaHitContainer");
  if(geahits == NULL) {
    std::cout << "No TMpcExGeaHitContainer! ... You need to load mMpcExCreateNodeTree" << std::endl; 
    return ABORTRUN;
  }

  //this is the real data pedestals and mip peak positions
  TMpcExCalibContainer *calibMap = findNode::getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(!calibMap){
    std::cout << "No TMpcExCalibContainer! ... You need to load mMpcExCreateNodeTree" << std::endl; 
    return ABORTRUN;
  }

  //this will be the digitized hits
  MpcExRawHit *rawhits = findNode::getClass<MpcExRawHit>(topNode,"MpcExRawHit");
  if(rawhits == NULL){
    std::cout << "No MpcExRawHit! ... You need to load mMpcExCreateNodeTree" << std::endl; 
    return ABORTRUN;
  }

  // pisaheader to store saturated minipad information
  int nLowSat[2] = {0,0}; 
  float eLowSat[2] = {0.0,0.0}; 
  MpcExPISAEventHeader *pisaheader = findNode::getClass<MpcExPISAEventHeader>(topNode,"MpcExPISAEventHeader");
  if (pisaheader == NULL) {
    std::cout<<PHWHERE<<" I could not find MpcExPISAEventHeader"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you are reading a simulated PISAEvent file"<<std::endl;
    //return ABORTRUN;
  } 

  MpcExMapper *mapper = MpcExMapper::instance();

  //I'm going to keep an internal std::set of the hits as I 
  //manipulate them. This saves lots of time since we need 
  //remove hits at the end or periodically for some reason
//  std::set<TMpcExHit*,TMpcExHitSort::SortByKey> hit_list;
  
  //use the map instead
  std::map<unsigned int,TMpcExHit*> hit_list;
  
  //  MpcExMapper *mapper = MpcExMapper::instance();
  //----------
  //Digitize the GEANT energy
  //----------
  for(unsigned int ghit=0; ghit<geahits->size(); ghit++){

    TMpcExGeaHit *geahit = geahits->getHit(ghit); 


    if(geahit == NULL){
      std::cout<<PHWHERE<<" Something is seriously wrong. I cannot get the GEANT hit #"<<ghit<<std::endl;
      std::cout<<PHWHERE<<"BAILING"<<std::endl;
      return ABORTRUN;
    }

    unsigned int key = geahit->key();
    
    //add cutoff 
    TMpcExCalib *calib = calibMap->get(key);
//    if(geahit->e()*1000000.0 < calib->get_mip_corr_cutoff_position()) continue;

    TMpcExHit *hit = new TMpcExHit(key);
    
    float adclo = 0.0;
    float adchi = 0.0;
    float esat = 0.0;
    
    float e_smear = _energy_smear_width;
    if(_e_smear[key]>0 && _smear_mod==DIFF_SMEAR) e_smear = _e_smear[key];

    if((hit->layer()==0) || (hit->layer()==1)){
      adclo = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01/MpcExConstants::FIXED_HL_RATIO,
				       MpcExConstants::FIXED_LOW_PEDESTAL,
				       MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH,
				       geahit->e()*1000000.0*(1+_dice->Gaus(0,e_smear)),
				       _dice,
				       &esat);
      if(_default_cut_low_adc[key]<0 || _fit_mpv_cut_low_adc[key]<0){
        float default_cut = calib->get_mip_corr_cutoff_position();
	float fit_mpv_cut = calib->get_mip_corr_mpv();
	//ignore the pedestal width,so set the width small
	_default_cut_low_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01/MpcExConstants::FIXED_HL_RATIO,MpcExConstants::FIXED_LOW_PEDESTAL,0.0001,default_cut,_dice,0);
	_fit_mpv_cut_low_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01/MpcExConstants::FIXED_HL_RATIO,MpcExConstants::FIXED_LOW_PEDESTAL,0.0001,fit_mpv_cut,_dice,0);
      }
    }
    else{
      adclo = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27/MpcExConstants::FIXED_HL_RATIO,
				       MpcExConstants::FIXED_LOW_PEDESTAL,
				       MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH,
				       geahit->e()*1000000.0*(1+_dice->Gaus(0,e_smear)),
				       _dice,
				       &esat);
       if(_default_cut_low_adc[key]<0 || _fit_mpv_cut_low_adc[key]<0){
         float default_cut = calib->get_mip_corr_cutoff_position();
         float fit_mpv_cut = calib->get_mip_corr_mpv();
	 //ignore the pedestal width,so set the width small
         _default_cut_low_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27/MpcExConstants::FIXED_HL_RATIO,MpcExConstants::FIXED_LOW_PEDESTAL,0.0001,default_cut,_dice,0);
         _fit_mpv_cut_low_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27/MpcExConstants::FIXED_HL_RATIO,MpcExConstants::FIXED_LOW_PEDESTAL,0.0001,fit_mpv_cut,_dice,0);
       }
    }

    if(makeHisto) _histo_low->Fill(key,adclo);
    if(esat>0.0) {
      unsigned int iarm = mapper->get_arm(key); 
      nLowSat[iarm]++; 
      eLowSat[iarm] += esat;
    }

    if((hit->layer()==0) || (hit->layer()==1)){
      adchi = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01,
				       MpcExConstants::FIXED_HIGH_PEDESTAL,
				       MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH,
				       geahit->e()*1000000.0*(1+_dice->Gaus(0,e_smear)),
				       _dice,
				       &esat);
      if(_default_cut_high_adc[key]<0 || _fit_mpv_cut_high_adc[key]<0){
        float default_cut = calib->get_mip_corr_cutoff_position();
	float fit_mpv_cut = calib->get_mip_corr_mpv();
	//ignore the pedestal width,so set the width small
	_default_cut_high_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01,MpcExConstants::FIXED_HIGH_PEDESTAL,0.0001,default_cut,_dice,0);
	_fit_mpv_cut_high_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L01,MpcExConstants::FIXED_HIGH_PEDESTAL,0.0001,fit_mpv_cut,_dice,0);
      }

    }
    else{
      adchi = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27,
				       MpcExConstants::FIXED_HIGH_PEDESTAL,
				       MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH,
				       geahit->e()*1000000.0*(1+_dice->Gaus(0,e_smear)),
				       _dice,
				       &esat);
      if(_default_cut_high_adc[key]<0 || _fit_mpv_cut_high_adc[key]<0){
        float default_cut = calib->get_mip_corr_cutoff_position();
	float fit_mpv_cut = calib->get_mip_corr_mpv();
	//ignore the pedestal width,so set the width small
	_default_cut_high_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27,MpcExConstants::FIXED_HIGH_PEDESTAL,0.0001,default_cut,_dice,0);
	_fit_mpv_cut_high_adc[key] = MpcExConstants::make_adc(MpcExConstants::FIXED_MIP_L27,MpcExConstants::FIXED_HIGH_PEDESTAL,0.0001,fit_mpv_cut,_dice,0);
      }
    }
    if(makeHisto) _histo_high->Fill(key,adchi);

    hit->set_low(adclo);
    hit->set_high(adchi);
    
    hit->set_status_low(TMpcExHit::ADC);
    hit->set_status_high(TMpcExHit::ADC);

    hit_list.insert(std::pair<int,TMpcExHit*>(key,hit));

  }//hit loop

  //OK, now we can make a "zero suppression" based on the DCM threshold

  perform_DCM_suppress(hit_list,rawhits);

  std::map<unsigned int,TMpcExHit*>::iterator hitr = hit_list.begin();
  for(; hitr!=hit_list.end(); ++hitr){
    TMpcExHit *digihit = hitr->second;
    delete digihit;
    digihit = NULL;
  }
  hit_list.clear();

  //add everything to the vector

  if(pisaheader){
    pisaheader->setnLowSat(0,nLowSat[0]); 
    pisaheader->seteLowSat(0,eLowSat[0]/1000000.0); //convert from keV to GeV 
    pisaheader->setnLowSat(1,nLowSat[1]); 
    pisaheader->seteLowSat(1,eLowSat[1]/1000000.0); //convert from keV to GeV
  }
  
  //std::cout << nLowSat[0] << " " << eLowSat[0] << std::endl; 
  //std::cout << nLowSat[1] << " " << eLowSat[1] << std::endl << std::endl; 

  return EVENT_OK;
}

int mMpcExMyDigitizeHits::End(PHCompositeNode *topNode)
{

  if(makeHisto){
    outputfile->Write();
    outputfile->Close();
  }
  
  return 0;
}

void mMpcExMyDigitizeHits::perform_DCM_suppress(std::map<unsigned int,TMpcExHit*>& hit_list,MpcExRawHit* rawhits){
  
  std::map<unsigned int,TMpcExHit*>::iterator hitr = hit_list.begin();
  std::vector<unsigned int> raw;
  for(; hitr!=hit_list.end(); ++hitr){
    TMpcExHit *digihit = hitr->second;

    unsigned short threshold_low = MpcExConstants::FIXED_LOW_PEDESTAL + 
      MpcExConstants::NORTH_DCM_LOW_SIGMA_CUT*MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH; 
    unsigned short threshold_high = MpcExConstants::FIXED_HIGH_PEDESTAL + 
      MpcExConstants::NORTH_DCM_HIGH_SIGMA_CUT*MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH; 
    
    if(digihit->arm()==0) {
      threshold_low = MpcExConstants::FIXED_LOW_PEDESTAL + 
	MpcExConstants::SOUTH_DCM_LOW_SIGMA_CUT*MpcExConstants::FIXED_LOW_PEDESTAL_WIDTH; 
      threshold_high = MpcExConstants::FIXED_HIGH_PEDESTAL + 
	MpcExConstants::SOUTH_DCM_HIGH_SIGMA_CUT*MpcExConstants::FIXED_HIGH_PEDESTAL_WIDTH;
    }

    unsigned short dual_threshold_low  = threshold_low;
    unsigned short dual_threshold_high = threshold_high;
    
    int key = digihit->key();
    int chain = digihit->chain();
    int dual_key = key + 2*MpcExConstants::NCHIPS_PER_CHAIN*MpcExConstants::NROCBONDS_PER_CHIP;
    if(chain ==2 || chain==3) dual_key = key - 2*MpcExConstants::NCHIPS_PER_CHAIN*MpcExConstants::NROCBONDS_PER_CHIP;

    if(_sup_mod == DEFAULT_CUT_SUP || _sup_mod == DEFAULT_CUT_SUP_REAL){
      unsigned short low_cut_off = _default_cut_low_adc[key];
      unsigned short high_cut_off = _default_cut_high_adc[key];
      if(low_cut_off > threshold_low) threshold_low = low_cut_off;
      if(high_cut_off > threshold_high) threshold_high = high_cut_off; 
      
      low_cut_off = _default_cut_low_adc[dual_key];
      high_cut_off = _default_cut_high_adc[dual_key];
      if(low_cut_off  > dual_threshold_low ) dual_threshold_low  = low_cut_off;
      if(high_cut_off > dual_threshold_high) dual_threshold_high = high_cut_off; 
    }

    if(_sup_mod == FIT_MPV_CUT_SUP || _sup_mod == FIT_MPV_CUT_SUP_REAL){
      unsigned short low_cut_off = _fit_mpv_cut_low_adc[key];
      unsigned short high_cut_off = _fit_mpv_cut_high_adc[key];
      if(low_cut_off > threshold_low) threshold_low = low_cut_off;
      if(high_cut_off > threshold_high) threshold_high = high_cut_off; 
      
      low_cut_off = _fit_mpv_cut_low_adc[dual_key];
      high_cut_off = _fit_mpv_cut_high_adc[dual_key];
      if(low_cut_off  > dual_threshold_low ) dual_threshold_low  = low_cut_off;
      if(high_cut_off > dual_threshold_high) dual_threshold_high = high_cut_off; 
    }
   
    bool dual_high_pass = false;
    bool dual_low_pass  = false;

    if(_sup_mod==FIX_SUP_REAL || _sup_mod==DEFAULT_CUT_SUP_REAL || _sup_mod==FIT_MPV_CUT_SUP_REAL){
      if(hit_list.find(dual_key)!=hit_list.end()){
        TMpcExHit* dual_digihit = hit_list[dual_key];
	if(dual_digihit->high() > dual_threshold_high) dual_high_pass = true;
	if(dual_digihit->low()  > dual_threshold_low ) dual_low_pass  = true;
      }
    }

    //turn this into the bit packed raw hit if hit would have passed the DCM threshold
    unsigned short adclow = 0; 
    unsigned short adchigh = 0;
    //minipad by minipad suppression
    //

    if(digihit->low()>threshold_low || dual_low_pass){
      float low = digihit->low();
      if(low<0) low = 0;
      adclow = low; 
      // histogram the ADC's
      if(makeHisto) _histo_low_dcm->Fill(digihit->key(),adclow);
    }
    if(digihit->high()>threshold_high || dual_high_pass){
      float high = digihit->high();
      if(high<0) high = 0;
      adchigh = high; 
      if(makeHisto) _histo_high_dcm->Fill(digihit->key(),adchigh);
    }

    // Add the hit if one of the two gains passes threshold
    if( (adchigh>0) || (adclow>0) ){
      unsigned short adc = (adchigh << 8) + adclow;
      unsigned int val = digihit->key() << 16;
      val |= adc;
      raw.push_back(val);
    }

  }//loop over hits
  
  rawhits->fillfromvector(raw); 
}

