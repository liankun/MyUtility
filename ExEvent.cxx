#include "ExEvent.h"
#include <TMpcExShower.h>
#include <TMpcExSingleTrackPi0.h>
#include <MpcMap.h>
#include <iostream>
#include <TClonesArray.h>
#include <string>

ClassImp(ExHit);
ClassImp(ExShower);
ClassImp(ExEvent);
ClassImp(ExEventHeader);

ExHit::~ExHit(){}

void ExShower::AddExHit(ExHit* hit){
  _ex_hits.push_back(hit);
}

void ExShower::ResetExShower(){
  _arm = -9999;
  _first_layer = -9999;
  _fired_layers = -9999;
  _patID = -9999;
  _calib_ok = -9999;
  _centrality = -9999;
  for(int i=0;i<2;i++){
    _x_hight[i] = -9999;
    _y_hight[i] = -9999;
    _x_pk[i] = -9999;
    _y_pk[i] = -9999;
  }
  _pi0_mass = -9999;
  _vertex = -9999;
  _cor_mpcexE = -9999;
  _raw_mpcexE = -9999;
  _cor_mpcE = -9999;
  _total_e = -9999;
  _hsx = -9999;
  _hsy = -9999;
  _rms_hsx = -9999;
  _rms_hsy = -9999;
  _disp_hsx = -9999;
  _disp_hsy = -9999;
  for(int i=0;i<8;i++){
    _layer_hsx[i] = -9999;
    _layer_hsy[i] = -9999;
    _layer_hsx_rms[i] = -9999;
    _layer_hsy_rms[i] = -9999;
    _layer_hsx_disp[i] = -9999;
    _layer_hsy_disp[i] = -9999;
    _layer_e[i] = -9999;
  }

  for(int i=0;i<7;i++){
    for(int j=0;j<7;j++){
      _mpc7x7_ch[i][j] = -9999;
      _mpc7x7_e[i][j] = -9999;
      _mpc7x7_tof[i][j] = -9999;
    }
  }

  _fired_towerN3x3 = -9999;
  _fired_towerN5x5 = -9999;
  _mpcE3x3 = -9999;
  _px = -9999;
  _py = -9999;
  _pz = -9999;
  _phi = -9999;
  _eta = -9999;
  _fit_slope = -9999;
  _fit_intercept = -9999;
  _fit_chi2 = -9999;
  for(unsigned int i=0;i<_ex_hits.size();i++){
    if(_ex_hits[i]) delete _ex_hits[i];
  }
  _ex_hits.clear();
}

void ExShower::Clear(Option_t* option){
  ResetExShower();
}

void ExShower::InitShower(){
  _arm = -9999;
  _first_layer = -9999;
  _fired_layers = -9999;
  _patID = -9999;
  _calib_ok = -9999;
  _centrality = -9999;
  for(int i=0;i<2;i++){
    _x_hight[i] = -9999;
    _y_hight[i] = -9999;
    _x_pk[i] = -9999;
    _y_pk[i] = -9999;
  }
  _pi0_mass = -9999;
  _vertex = -9999;
  _cor_mpcexE = -9999;
  _raw_mpcexE = -9999;
  _cor_mpcE = -9999;
  _total_e = -9999;
  _hsx = -9999;
  _hsy = -9999;
  _rms_hsx = -9999;
  _rms_hsy = -9999;
  _disp_hsx = -9999;
  _disp_hsy = -9999;
  for(int i=0;i<8;i++){
    _layer_hsx[i] = -9999;
    _layer_hsy[i] = -9999;
    _layer_hsx_rms[i] = -9999;
    _layer_hsy_rms[i] = -9999;
    _layer_hsx_disp[i] = -9999;
    _layer_hsy_disp[i] = -9999;
    _layer_e[i] = -9999;
  }

  for(int i=0;i<7;i++){
    for(int j=0;j<7;j++){
      _mpc7x7_ch[i][j] = -9999;
      _mpc7x7_e[i][j] = -9999;
      _mpc7x7_tof[i][j] = -9999;
    }
  }

  _fired_towerN3x3 = -9999;
  _fired_towerN5x5 = -9999;
  _mpcE3x3 = -9999;
  _px = -9999;
  _py = -9999;
  _pz = -9999;
  _phi = -9999;
  _eta = -9999;
  _fit_slope = -9999;
  _fit_intercept = -9999;
  _fit_chi2 = -9999;
  _ex_hits.clear();
}

ExShower::ExShower(){
  InitShower();
}

ExShower::ExShower(TMpcExShower* shower,TMpcExSingleTrackPi0* st_pi0){
  if(shower && st_pi0){  
    InitShower();
    SetExShower(shower);
    SetSTPi0(st_pi0,_vertex);
  }
}

ExShower::ExShower(TMpcExShower* shower){
  if(shower){
     InitShower();
     SetExShower(shower);
  }
}

ExShower::~ExShower(){
  ResetExShower();
}

void ExShower::SetExShower(TMpcExShower* shower){
  if(!shower){
    std::cout<<"ExShower::SetExShower Null shower input !"<<std::endl;
    return;
  }
  _calib_ok = shower->get_CalibEInRange();
  _arm=shower->get_arm();
  _first_layer=shower->get_first_layer();
  _fired_layers=shower->get_nlayers();
  _vertex = shower->get_vertex();
  _cor_mpcexE=shower->get_esum();
  _raw_mpcexE=shower->get_raw_esum();
  _cor_mpcE=shower->get_mpcECorr();
  _total_e=shower->get_roughTotE();
  _hsx=shower->get_hsx();
  _hsy=shower->get_hsy();
  _rms_hsx=shower->get_rms_hsx();
  _rms_hsy=shower->get_rms_hsy();
  _disp_hsx=shower->get_disp_hsx();
  _disp_hsy=shower->get_disp_hsy();
  _fired_towerN3x3=shower->get_mpcN3x3();
  _fired_towerN5x5=shower->get_mpcN5x5();
  _mpcE3x3=shower->get_mpcE3x3();
  _px=shower->get_Px();
  _py=shower->get_Py();
  _pz=shower->get_Pz();
  _phi=shower->get_phi();
  _eta=shower->get_eta();

//  _fit_slope=shower->get_fit_slope();
//  _fit_intercept=shower->get_fit_intercept();
//  _fit_chi2=shower->get_fit_chi2dof();
  
  for(int ilayer=0;ilayer<8;ilayer++){
    _layer_hsx[ilayer]=shower->get_hsx_layer(ilayer);
    _layer_hsy[ilayer]=shower->get_hsy_layer(ilayer);
    _layer_hsx_rms[ilayer]=shower->get_rms_hsx_layer(ilayer);
    _layer_hsy_rms[ilayer]=shower->get_rms_hsy_layer(ilayer);
    _layer_hsx_disp[ilayer]=shower->get_disp_hsx_layer(ilayer);
    _layer_hsy_disp[ilayer]=shower->get_disp_hsy_layer(ilayer);
    _layer_e[ilayer]=shower->get_e_layer(ilayer);
  }
}

void ExShower::SetMpc7x7Ch(int mpc7x7ch[7][7]){
  if(!mpc7x7ch){
    std::cout<<"ExShower::SetMpc7x7Ch : NUll array"<<std::endl;
    return;
  }
  for(int i=0;i<7;i++){
    for(int j=0;j<7;j++){
      _mpc7x7_ch[i][j] = mpc7x7ch[i][j];
    }
  }
}

void ExShower::SetMpc7x7Tof(float mpc7x7tof[7][7]){
  if(!mpc7x7tof){
    std::cout<<"ExShower::SetMpc7x7Tof : NUll array"<<std::endl;
    return;
  }
  for(int i=0;i<7;i++){
    for(int j=0;j<7;j++){
      _mpc7x7_tof[i][j] = mpc7x7tof[i][j];
    }
  }
}

void ExShower::SetMpc7x7E(float mpc7x7e[7][7]){
  if(!mpc7x7e){
    std::cout<<"ExShower::SetMpc7x7E : NUll array"<<std::endl;
    return;
  }
  for(int i=0;i<7;i++){
    for(int j=0;j<7;j++){
      _mpc7x7_e[i][j] = mpc7x7e[i][j];
    }
  }
}

void ExShower::SetSTPi0(TMpcExSingleTrackPi0* pi0,float vtx){
  //must fill after shower is set
  if(!pi0){
     std::cout<<"ExShower::SetSTPi0 : NULL ST Pi0"<<std::endl;
     return;
  }
  _patID = pi0->GetPeakPattern();
  _pi0_mass = pi0->GetMass(vtx);

  if(pi0->GetPeakPattern()==1){
    _x_hight[0]=pi0->GetPeakHeight(0,0);
    _x_hight[1]=pi0->GetPeakHeight(0,1);
    _y_hight[0]=pi0->GetPeakHeight(1,0);
    _y_hight[1]=pi0->GetPeakHeight(1,0);
    _x_pk[0]=pi0->GetPeakPosition(0,0);
    _x_pk[1]=pi0->GetPeakPosition(0,1);
    _y_pk[0]=pi0->GetPeakPosition(1,0);
    _y_pk[1]=pi0->GetPeakPosition(1,0);
  }
  
  if(pi0->GetPeakPattern()==2){
    _x_hight[0]=pi0->GetPeakHeight(0,0);
    _x_hight[1]=pi0->GetPeakHeight(0,0);
    _y_hight[0]=pi0->GetPeakHeight(1,0);
    _y_hight[1]=pi0->GetPeakHeight(1,1);
    _x_pk[0]=pi0->GetPeakPosition(0,0);
    _x_pk[1]=pi0->GetPeakPosition(0,0);
    _y_pk[0]=pi0->GetPeakPosition(1,0);
    _y_pk[1]=pi0->GetPeakPosition(1,1);
  }

  if(pi0->GetPeakPattern()==3){
    _x_hight[0]=pi0->GetPeakHeight(0,0);
    _x_hight[1]=pi0->GetPeakHeight(0,1);
    _y_hight[0]=pi0->GetPeakHeight(1,0);
    _y_hight[1]=pi0->GetPeakHeight(1,1);
    _x_pk[0]=pi0->GetPeakPosition(0,0);
    _x_pk[1]=pi0->GetPeakPosition(0,1);
    _y_pk[0]=pi0->GetPeakPosition(1,0);
    _y_pk[1]=pi0->GetPeakPosition(1,1);
  }
}

ExEvent::ExEvent(){
  _centrality = -9999;
  _vertex = -9999;
  _run_number = -9999;
  _nhits_north = -9999;
  _nhits_south = -9999;
  _trigger = -9999;
  _ex_showers.clear();
}

ExEvent::ExEvent(ExEventHeader header){
  _centrality = header.GetCentrality();
  _vertex = header.GetVertex();
  _run_number = header.GetRunNumber();
  _nhits_north = header.GetNorthNhits();
  _nhits_south = header.GetSouthNhits();
  _trigger = header.GetTrigger();
  _ex_showers.clear();
}

void ExEvent::Clear(Option_t*){
  Reset();
}

ExEvent::~ExEvent(){
  Reset();
}

int ExEvent::GetTrigger(const char* trgName){
  //1 stands for "BBCLL1(>0 tubes)"
  //2 stands for "BBCLL1(>0 tubes) novertex"
  //4 stands for "BBCLL1(>0 tubes) narrowvtx"
  //8 stands for "BBCLL1(>0 tubes)_central_narrowvtx"
  //16 stands for "MPC_N_B"
  
  if(_trigger == -9999) return 0;

  if(!strcmp(trgName,"BBCLL1(>0 tubes)")){
    return (_trigger&1)==1?1:0;
  }

  if(!strcmp(trgName,"BBCLL1(>0 tubes) novertex")){
    return (_trigger&2)==2?1:0;
  }
  
  if(!strcmp(trgName,"BBCLL1(>0 tubes) narrowvtx")){
    return (_trigger&4)==4?1:0;
  }

  if(!strcmp(trgName,"BBCLL1(>0 tubes)_central_narrowvtx")){
    return (_trigger&8)==8?1:0;
  }

  if(!strcmp(trgName,"MPC_N_B")){
    return (_trigger&16)==16?1:0;
  }
  
  std::cout<<"ExEvent::GetTrigger NO trigger matched !!"<<std::endl;
  return -9999;
}

void ExEvent::AddExShower(ExShower* ex_shower){
  _ex_showers.push_back(ex_shower);
}

void ExEvent::Reset(){
  _centrality = -9999;
  _vertex = -9999;
  _run_number = -9999;
  _nhits_north = -9999;
  _nhits_south = -9999;
  _trigger = -9999;
  for(unsigned int i = 0;i<_ex_showers.size();i++){
    if(_ex_showers[i]) delete _ex_showers[i];
  }
  _ex_showers.clear();
}
