#include "TMpcExShower.h"
#include "JohnCalibrateShowerEnergy.h"
#include "recoConsts.h"
#include <phool.h>
#include <iostream>
#include <cstdio>
#include <math.h> 

#include <TGraph.h>

#include "MpcExTSpline1.h"

#include "TMVA/Types.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"
#include "TMVA/MethodCuts.h"

#include "ExEvent.h"

using namespace std;

// The configuration data (xml files stored as char strings)

#include "ml_training_data_bdtg_arm0.h"
#include "ml_training_data_bdtg_arm0_inner.h"
#include "ml_training_data_bdtg_arm0_outer.h"
#include "ml_training_data_bdtg_arm0_E33_0.h"
#include "ml_training_data_bdtg_arm1.h"
#include "ml_training_data_bdtg_arm1_inner.h"
#include "ml_training_data_bdtg_arm1_outer.h"
#include "ml_training_data_bdtg_arm1_E33_0.h"

#include "ml_training_data_mlp_arm0.h"
#include "ml_training_data_mlp_arm0_inner.h"
#include "ml_training_data_mlp_arm0_outer.h"
#include "ml_training_data_mlp_arm0_E33_0.h"
#include "ml_training_data_mlp_arm1.h"
#include "ml_training_data_mlp_arm1_inner.h"
#include "ml_training_data_mlp_arm1_outer.h"
#include "ml_training_data_mlp_arm1_E33_0.h"
#include "ml_training_data_bdtg_mpcex_only.h"
#include "ml_training_data_bdtg_photon_frac.h"
#include "ml_training_data_bdtg_pi0_frac.h"


JohnCalibrateShowerEnergy* JohnCalibrateShowerEnergy::_instance = NULL;  

JohnCalibrateShowerEnergy* JohnCalibrateShowerEnergy::instance() {
  if(_instance == NULL){
    _instance = new JohnCalibrateShowerEnergy();
  }
  return _instance;
}

JohnCalibrateShowerEnergy::JohnCalibrateShowerEnergy() {
  
  SetSplines(); 

  // Wish we could use C++11....

  double param00[4] = {2.941567465860e+00,1.493607399708e+00,1.542617427282e+00,3.310008658010e+00}; 
  double param01[4] = {2.941573831834e+00,1.493611524392e+00,1.542609187118e+00,3.310006227518e+00}; 
  double param02[4] = {2.880229526265e+00,1.498297802205e+00,1.599975590334e+00,4.123996850142e+00}; 
  double param03[4] = {2.941568673143e+00,1.493607422569e+00,1.542617670303e+00,3.310006995255e+00}; 
  double param10[4] = {2.751730099833e+00,1.252810006016e+00,1.432092860632e+00,3.375344302574e+00}; 
  double param11[4] = {2.885577045338e+00,1.346241892361e+00,1.491012104003e+00,3.317237917654e+00}; 
  double param12[4] = {2.853701763894e+00,1.328885071984e+00,1.564003688748e+00,3.692986165997e+00}; 
  double param13[4] = {2.885578239533e+00,1.250837340886e+00,1.400395083783e+00,3.312965698623e+00}; 

  for(int i=0; i<4; i++){
    CNST[0][0][i] = param00[i];
    CNST[0][1][i] = param01[i];
    CNST[0][2][i] = param02[i];
    CNST[0][3][i] = param03[i];
    CNST[1][0][i] = param10[i];
    CNST[1][1][i] = param11[i];
    CNST[1][2][i] = param12[i];
    CNST[1][3][i] = param13[i];
  }

  recoConsts *myrc = recoConsts::instance();
  
  if(myrc->get_IntFlag("MPCEX_ML_USE_MLP",0x0)){
     methodType = TMVA::Types::kMLP; 
    std::cout<<PHWHERE<<" Using MLP machine learning for total energy."<<std::endl;
  }
  else{
    methodType = TMVA::Types::kBDT; 
    std::cout<<PHWHERE<<" Using BDT machine learning for total energy."<<std::endl;
  }

  SetupRegression(); 

  ml_resum = 0.0; 
  ml_Cangle = 0.0; 
  ml_E33 = 0.0; 
  ml_in33 = 0.0; 
  ml_insat = 0.0; 
  ml_ifl = 0.0; 
  ml_ffx = 0.0; 
  ml_ffy = 0.0; 
  ml_vtx = 0.0; 

  for(int ilayer=0;ilayer<8;ilayer++){
    _ml_layer_e[ilayer] = 0;
    _ml_layer_rms[ilayer] = 0;
  }
  _ml_reco_e = 0;
  _ml_shower_rms = 0;
  _ml_nhits = 0;
  _ml_first_layer = 0;
  _ml_nsat = 0;
  _ml_hsx = 0;
  _ml_hsy = 0;

}

JohnCalibrateShowerEnergy::~JohnCalibrateShowerEnergy(){

  // delete the allocated splines
  // (splines delete the graph)

  for(int iarm=0; iarm<2; iarm++){
    for(int ilyr=0; ilyr<5; ilyr++){
      delete ex_calSpline[iarm][ilyr];
      if(ilyr<4) {
	delete mpc_calSpline[iarm][ilyr];
	delete comb_calSpline[iarm][ilyr];
      }
    }
    delete al_calSpline[iarm]; 
  }

  // delete the TMVA readers

  for(int i=0; i<2; i++){
    for(int j=0; j<3; j++){
      delete reader[i][j]; 
    }
    delete reader_E33_0[i]; 
  }

}

void JohnCalibrateShowerEnergy::CalibrateEnergy(TMpcExShower *shower_v1, bool use_cutoff){

  DoCalibrateEnergy(shower_v1,shower_v1->get_raw_esum(),shower_v1->get_mpcE3x3(use_cutoff),NULL,NULL,NULL,true,NULL,false,0); 

}

void JohnCalibrateShowerEnergy::CalibrateEnergyDecoupledPair(TMpcExShower *shower_v1, int nShared){

  DoCalibrateEnergy(shower_v1,shower_v1->get_raw_esum(),shower_v1->get_mpcE3x3(false),NULL,NULL,NULL,true,NULL,true,nShared); 

}

float JohnCalibrateShowerEnergy::RecalCalibratedEnergy(TMpcExShower *shower_v1, float MpcEx_E, float MPC_E, 
							float *MPCXE, float *ALPLTE, float *MPCE, int *CalOKFlag, bool pairFlag){

  return DoCalibrateEnergy(shower_v1,MpcEx_E,MPC_E,MPCXE,ALPLTE,MPCE,false,CalOKFlag, pairFlag);

}

float JohnCalibrateShowerEnergy::DoCalibrateEnergy(TMpcExShower *shower_v1, float MpcEx_E, float MPC_E, 
						    float *MPCXE, float *ALPLTE, float *MPCE, 
						    bool setShower, int *CalOKFlag, bool pairFlag, 
						    int nShared){
  // The "pairFlag" used to activate a special 
  // low-energy MPC calibration, but is 
  // no deprecated and unused. 
  // 12/12/2017 JGL

  // Flags for linearity corrections

  bool totLinOK = false; 
  
  int iarm = shower_v1->get_arm();  
  int lfl_bin = shower_v1->get_first_layer();
  if(lfl_bin>4) lfl_bin = 4; 

  double CAngle = 1.0/sqrt(pow(shower_v1->get_hsx(),2) + pow(shower_v1->get_hsy(),2) + 1);

  float ELOW_ENERGY_CORR =  ex_calSpline[iarm][lfl_bin]->Eval(MpcEx_E*CAngle)/CAngle; 

  if(setShower) shower_v1->set_esum(ELOW_ENERGY_CORR);
  if(MPCXE) *MPCXE = ELOW_ENERGY_CORR; 

  int spline_bin = 1; 
  if(isInnerTower(shower_v1->get_mpcPeakix(),shower_v1->get_mpcPeakiy(),shower_v1->get_arm())) 
    spline_bin = 2;
  else if(isOuterTower(shower_v1->get_mpcPeakix(),shower_v1->get_mpcPeakiy(),shower_v1->get_arm()))
    spline_bin = 3; 
  else if(shower_v1->get_mpcN3x3() == 8)
    spline_bin = 0; 
  else if(shower_v1->get_mpcN3x3() == 9)
    spline_bin = 1; 
  else
    spline_bin = -1; 
      
  float E33_CORR = 0.0; 
  if(isnan(MPC_E)){
    cout << PHWHERE << " MPC energy is NAN = " << MPC_E << endl; 
  }
  else if(MPC_E>0.0){
    if((spline_bin>=0)&&(spline_bin<4)){
      E33_CORR = mpc_calSpline[iarm][spline_bin]->Eval(MPC_E);          
    }
  }
 
  if(setShower) shower_v1->set_mpcECorr(E33_CORR);    
  if(MPCE) *MPCE = E33_CORR; 

  // Al plate energy

  // factor of 5.0 makes correlation closer to 1:1
  float AL_PLATE_ENERGY =  al_calSpline[iarm]->Eval(MpcEx_E*5.0*CAngle)/CAngle;

  if(setShower) shower_v1->set_AlPlateEnergy(AL_PLATE_ENERGY); 
  if(ALPLTE) *ALPLTE = AL_PLATE_ENERGY; 

  // OLD spline-based calibration
  float TOT_E_CORR = 0.0; 

  // if((spline_bin>=0) && (spline_bin<=3)) {
  //   TOT_E_CORR = ((CNST[iarm][spline_bin][0]*ELOW_ENERGY_CORR + 
  // 		   CNST[iarm][spline_bin][1]*E33_CORR + AL_PLATE_ENERGY)/CNST[iarm][spline_bin][2])- CNST[iarm][spline_bin][3]; 
  //   totLinOK = true; 
  // }
  // else
  //   totLinOK = false; 

  // NEW MLP- or BDT-based regression energy calibration
    
  int lidx = 0;
  if(isInnerTower(shower_v1->get_mpcPeakix(),shower_v1->get_mpcPeakiy(),shower_v1->get_arm())) lidx = 1; 
  if(isOuterTower(shower_v1->get_mpcPeakix(),shower_v1->get_mpcPeakiy(),shower_v1->get_arm())) lidx = 2; 

  ml_resum = MpcEx_E; 
  ml_Cangle = CAngle; 
  ml_E33 = MPC_E;
  ml_in33 = shower_v1->get_mpcN3x3();
  ml_insat = shower_v1->get_n_sat_minipads();
  ml_ifl = shower_v1->get_first_layer(); 
  if(ml_ifl>=4) ml_ifl = 4; 

  ml_vtx = shower_v1->get_vertex(); 
  ml_ffx = shower_v1->get_hsx()*200.0; 
  ml_ffy = shower_v1->get_hsy()*200.0; 

  if(MPC_E>0.0){
    TOT_E_CORR = (reader[shower_v1->get_arm()][lidx]->EvaluateRegression( method[shower_v1->get_arm()][lidx] ))[0];
  }
  else{
    TOT_E_CORR = (reader_E33_0[shower_v1->get_arm()]->EvaluateRegression( method_E33_0[shower_v1->get_arm()] ))[0];
  }

  totLinOK = true; 
  
  if(setShower) shower_v1->set_roughTotE(TOT_E_CORR);

  // Set calibration validity flag

  if( ((spline_bin>=0) && (spline_bin<=3)) && totLinOK &&  
      (shower_v1->get_first_layer()<=4) && (TOT_E_CORR>0.0) ){
    
    // E33=0 showers only good when in interior of MPC-EX
    
    if( (MPC_E>0.0) || ((MPC_E==0.0)&&((spline_bin==0)||(spline_bin==1))) ){
      if(setShower) shower_v1->set_CalibEInRange(1);
      if(CalOKFlag) *CalOKFlag = 1; 
    }
    else{
      if(setShower) shower_v1->set_CalibEInRange(0);
      if(CalOKFlag) *CalOKFlag = 0; 
    }

  }
  else {
    if(setShower) shower_v1->set_CalibEInRange(0);
    if(CalOKFlag) *CalOKFlag = 0;  
  }

  return TOT_E_CORR; 

}

double JohnCalibrateShowerEnergy::DoCalibrateEnergy(int arm,double MpcEx_E,double MPC_E,double vtx,
                          double hsx,double hsy,int pk_ix,int pk_iy,
			  int first_layer, int MpcN3x3,int n_sat_mpads){
  bool totLinOK = false; 
  
//  int iarm = arm;  
  int lfl_bin = first_layer;
  if(lfl_bin>4) lfl_bin = 4; 

  double CAngle = 1.0/sqrt(pow(hsx,2) + pow(hsy,2) + 1);

//  float ELOW_ENERGY_CORR =  ex_calSpline[iarm][lfl_bin]->Eval(MpcEx_E*CAngle)/CAngle; 

  int spline_bin = 1; 
  if(isInnerTower(pk_ix,pk_iy,arm)) 
    spline_bin = 2;
  else if(isOuterTower(pk_ix,pk_iy,arm))
    spline_bin = 3; 
  else if(MpcN3x3 == 8)
    spline_bin = 0; 
  else if(MpcN3x3 == 9)
    spline_bin = 1; 
  else
    spline_bin = -1; 
      
//  float E33_CORR = 0.0; 
//  if(isnan(MPC_E)){
//    cout << PHWHERE << " MPC energy is NAN = " << MPC_E << endl; 
//  }
//  else if(MPC_E>0.0){
//    if((spline_bin>=0)&&(spline_bin<4)){
//      E33_CORR = mpc_calSpline[iarm][spline_bin]->Eval(MPC_E);          
//    }
//  }
 
  // Al plate energy

  // factor of 5.0 makes correlation closer to 1:1
//  float AL_PLATE_ENERGY =  al_calSpline[iarm]->Eval(MpcEx_E*5.0*CAngle)/CAngle;


  // OLD spline-based calibration
  float TOT_E_CORR = 0.0; 

  // if((spline_bin>=0) && (spline_bin<=3)) {
  //   TOT_E_CORR = ((CNST[iarm][spline_bin][0]*ELOW_ENERGY_CORR + 
  // 		   CNST[iarm][spline_bin][1]*E33_CORR + AL_PLATE_ENERGY)/CNST[iarm][spline_bin][2])- CNST[iarm][spline_bin][3]; 
  //   totLinOK = true; 
  // }
  // else
  //   totLinOK = false; 

  // NEW MLP- or BDT-based regression energy calibration
    
  int lidx = 0;
  if(isInnerTower(pk_ix,pk_iy,arm)) lidx = 1; 
  if(isOuterTower(pk_ix,pk_iy,arm)) lidx = 2; 

  ml_resum = MpcEx_E; 
  ml_Cangle = CAngle; 
  ml_E33 = MPC_E;
  ml_in33 = MpcN3x3;
  ml_insat = n_sat_mpads;
  ml_ifl = first_layer; 
  if(ml_ifl>=4) ml_ifl = 4; 

  ml_vtx = vtx; 
  ml_ffx = hsx*200.0; 
  ml_ffy = hsy*200.0; 

  if(MPC_E>0.0){
    TOT_E_CORR = (reader[arm][lidx]->EvaluateRegression( method[arm][lidx] ))[0];
  }
  else{
    TOT_E_CORR = (reader_E33_0[arm]->EvaluateRegression( method_E33_0[arm] ))[0];
  }

  totLinOK = true; 
  

  // Set calibration validity flag
  bool cal_ok = false;

  /*
  std::cout<<"spline_bin "<<spline_bin
      <<" first_layer "<<first_layer
      <<" TOT_E_CORR "<<TOT_E_CORR
      <<" MPC_E "<<MPC_E
      <<std::endl;
  */

  if( ((spline_bin>=0) && (spline_bin<=3)) && totLinOK &&  
      (first_layer<=4) && (TOT_E_CORR>0.0) ){
    
    // E33=0 showers only good when in interior of MPC-EX
    
    if( (MPC_E>0.0) || ((MPC_E==0.0)&&((spline_bin==0)||(spline_bin==1))) ){
      cal_ok = true;
    }
  }
  
  if(!cal_ok) TOT_E_CORR = -1;

  return TOT_E_CORR; 

}


double JohnCalibrateShowerEnergy::CalibrateUseMpcEx(double layer_e[8],double shower_rms,int nhits,
		                                    int first_layer,int nsat,double hsx,double hsy){
  _ml_layer_e[0] = layer_e[0];
  _ml_layer_e[1] = layer_e[1];
  _ml_layer_e[2] = layer_e[2];
  _ml_layer_e[3] = layer_e[3];
  _ml_layer_e[4] = layer_e[4];
  _ml_layer_e[5] = layer_e[5];
  _ml_layer_e[6] = layer_e[6];
  _ml_layer_e[7] = layer_e[7];
  _ml_shower_rms = shower_rms;
  _ml_nhits = nhits;
  _ml_first_layer = first_layer;
  _ml_nsat = nsat;
  _ml_hsx = hsx;
  _ml_hsy = hsy;

  double E = (_mpcex_only_reader->EvaluateRegression(_mpcex_only_method))[0];

  return E;
}


void JohnCalibrateShowerEnergy::SetSplines()
{

  int n_ex_sppoints[2][5]; 
  double mpcex_spdata[2][5][2][400]; 

  int n_mpc_sppoints[2][4]; 
  double mpc_spdata[2][4][2][200]; 

  int n_al_sppoints[2]; 
  double al_spdata[2][2][205]; 

#include "ShowerSplineData.C"

  // Create the first order splines

  for(int iarm=0; iarm<2; iarm++){
    for(int ilyr=0; ilyr<5; ilyr++){

      double test_data_x[400]; 
      double test_data_y[400]; 

      for(int i=0; i<n_ex_sppoints[iarm][ilyr]; i++){
        test_data_x[i] = mpcex_spdata[iarm][ilyr][0][i]; 
        test_data_y[i] = mpcex_spdata[iarm][ilyr][1][i]; 
      }

      ex_calSpline[iarm][ilyr] = new MpcExTSpline1(new TGraph(n_ex_sppoints[iarm][ilyr],test_data_x,test_data_y)); 

    }
  }

  for(int iarm=0; iarm<2; iarm++){
    for(int ilyr=0; ilyr<4; ilyr++){

      double test_data_x[200]; 
      double test_data_y[200]; 

      for(int i=0; i<n_mpc_sppoints[iarm][ilyr]; i++){
        test_data_x[i] = mpc_spdata[iarm][ilyr][0][i]; 
        test_data_y[i] = mpc_spdata[iarm][ilyr][1][i]; 
      }

      mpc_calSpline[iarm][ilyr] = new MpcExTSpline1(new TGraph(n_mpc_sppoints[iarm][ilyr],test_data_x,test_data_y));

    }
  }

  for(int iarm=0; iarm<2; iarm++){

      double test_data_x[205]; 
      double test_data_y[205]; 

      for(int i=0; i<n_al_sppoints[iarm]; i++){
        test_data_x[i] = al_spdata[iarm][0][i]; 
        test_data_y[i] = al_spdata[iarm][1][i]; 
      }

      al_calSpline[iarm] = new MpcExTSpline1(new TGraph(n_al_sppoints[iarm],test_data_x,test_data_y));

  }

  // Final Linearity Correction

  int n_comb_sppoints[2][4]; 
  double comb_spdata[2][4][2][400]; 

#include "LinearityCorrectionSplineData.C"

  for(int iarm=0; iarm<2; iarm++){
    for(int ilyr=0; ilyr<4; ilyr++){

      double test_data_x[400]; 
      double test_data_y[400]; 

      for(int i=0; i<n_comb_sppoints[iarm][ilyr]; i++){
        test_data_x[i] = comb_spdata[iarm][ilyr][0][i]; 
        test_data_y[i] = comb_spdata[iarm][ilyr][1][i]; 
      }

      comb_calSpline[iarm][ilyr] = new MpcExTSpline1(new TGraph(n_comb_sppoints[iarm][ilyr],test_data_x,test_data_y));

    }
  }

}

void JohnCalibrateShowerEnergy::SetupRegression()
{

  // Allocate the readers
  
  for(int i=0; i<2; i++){
    for(int j=0; j<3; j++){

      //cout << " create reader " << i << j << endl; 

      reader[i][j] = new TMVA::Reader( "!Color:!Silent" );   
 
      // Add variables
      
      reader[i][j]->AddVariable("resum",&ml_resum); 
      reader[i][j]->AddVariable("Cangle",&ml_Cangle); 
      reader[i][j]->AddVariable("insat",&ml_insat); 
      reader[i][j]->AddVariable("E33",&ml_E33); 
      reader[i][j]->AddVariable("in33",&ml_in33); 
      reader[i][j]->AddVariable("ifl",&ml_ifl); 
      reader[i][j]->AddVariable("vtx",&ml_vtx); 
      reader[i][j]->AddVariable("ffx",&ml_ffx); 
      reader[i][j]->AddVariable("ffy",&ml_ffy); 

      unsigned char *xmlstr = NULL; 

      switch(methodType){
      case(TMVA::Types::kBDT): 
	switch(i){
	case 0:
	  switch(j){
	  case 0:
	    xmlstr = TMVARegression_BDTG_weights_arm0_xml; 
	    break; 
	  case 1:
	    xmlstr = TMVARegression_BDTG_weights_arm0_inner_xml; 	  
	    break;
	  case 2:
	    xmlstr = TMVARegression_BDTG_weights_arm0_outer_xml; 
	    break; 
	  }
	  break; 
	case 1:
	  switch(j){
	  case 0:
	    xmlstr = TMVARegression_BDTG_weights_arm1_xml; 
	    break; 
	  case 1:
	    xmlstr = TMVARegression_BDTG_weights_arm1_inner_xml; 	  
	    break;
	  case 2:
	    xmlstr = TMVARegression_BDTG_weights_arm1_outer_xml; 
	    break; 
	  }
	  break; 
	}
	break; 
      case(TMVA::Types::kMLP):
	switch(i){
	case 0:
	  switch(j){
	  case 0:
	    xmlstr = TMVARegression_MLP_weights_arm0_xml; 
	    break; 
	  case 1:
	    xmlstr = TMVARegression_MLP_weights_arm0_inner_xml; 	  
	    break;
	  case 2:
	    xmlstr = TMVARegression_MLP_weights_arm0_outer_xml; 
	    break; 
	  }
	  break; 
	case 1:
	  switch(j){
	  case 0:
	    xmlstr = TMVARegression_MLP_weights_arm1_xml; 
	    break; 
	  case 1:
	    xmlstr = TMVARegression_MLP_weights_arm1_inner_xml; 	  
	    break;
	  case 2:
	    xmlstr = TMVARegression_MLP_weights_arm1_outer_xml; 
	    break; 
	  }
	  break; 
	}
	break; 

      default:
	// should never get here
	break; 
      }

      // Read training data

      method[i][j] = dynamic_cast<TMVA::MethodBase*>(reader[i][j]->BookMVA( methodType, (char *)xmlstr )); 

    }
  }


  for(int i=0; i<2; i++){

    reader_E33_0[i] = new TMVA::Reader( "!Color:!Silent" );   
 
    // Add variables
      
    reader_E33_0[i]->AddVariable("resum",&ml_resum); 
    reader_E33_0[i]->AddVariable("Cangle",&ml_Cangle); 
    reader_E33_0[i]->AddVariable("insat",&ml_insat); 
    reader_E33_0[i]->AddVariable("ifl",&ml_ifl); 
    reader_E33_0[i]->AddVariable("vtx",&ml_vtx); 
    reader_E33_0[i]->AddVariable("ffx",&ml_ffx); 
    reader_E33_0[i]->AddVariable("ffy",&ml_ffy); 

    // Read training data

    unsigned char *xmlstr = NULL; 

    switch(methodType){
    case(TMVA::Types::kBDT): 
      switch(i){
      case 0:
	xmlstr = TMVARegression_BDTG_weights_arm0_E33_0_xml; 
	break; 
      case 1:
	xmlstr = TMVARegression_BDTG_weights_arm1_E33_0_xml; 
	break; 
      }
      break; 
    case(TMVA::Types::kMLP):
      switch(i){
      case 0:
	xmlstr = TMVARegression_MLP_weights_arm0_E33_0_xml; 
	break; 
      case 1:
	xmlstr = TMVARegression_MLP_weights_arm1_E33_0_xml; 
	break; 
      }
      break;
    default:
      // should never get here
      break; 
    }

    method_E33_0[i] = dynamic_cast<TMVA::MethodBase*>(reader_E33_0[i]->BookMVA(methodType, (char *)xmlstr )); 

  }

  _mpcex_only_reader = new TMVA::Reader( "!Color:!Silent" );   
  _mpcex_only_reader->AddVariable("L0",&_ml_layer_e[0]);
  _mpcex_only_reader->AddVariable("L1",&_ml_layer_e[1]);
  _mpcex_only_reader->AddVariable("L2",&_ml_layer_e[2]);
  _mpcex_only_reader->AddVariable("L3",&_ml_layer_e[3]);
  _mpcex_only_reader->AddVariable("L4",&_ml_layer_e[4]);
  _mpcex_only_reader->AddVariable("L5",&_ml_layer_e[5]);
  _mpcex_only_reader->AddVariable("L6",&_ml_layer_e[6]);
  _mpcex_only_reader->AddVariable("L7",&_ml_layer_e[7]);
//  _mpcex_only_reader->AddVariable("showerRMS",&_ml_shower_rms);
//  _mpcex_only_reader->AddVariable("nhits",&_ml_nhits);
  _mpcex_only_reader->AddVariable("first_layer",&_ml_first_layer);
//  _mpcex_only_reader->AddVariable("nsat",&_ml_nsat);
  _mpcex_only_reader->AddVariable("hsx",&_ml_hsx);
  _mpcex_only_reader->AddVariable("hsy",&_ml_hsy);

  _mpcex_only_method = dynamic_cast<TMVA::MethodBase*>(_mpcex_only_reader->BookMVA(TMVA::Types::kBDT, (char *)TMVARegression_MpcExOnly_BDTG_weights_xml)); 

 _photon_frac_reader = new TMVA::Reader( "!Color:!Silent" );   
 _photon_frac_reader->AddVariable("L0",&_ml_layer_e[0]);
 _photon_frac_reader->AddVariable("L1",&_ml_layer_e[1]);
 _photon_frac_reader->AddVariable("L2",&_ml_layer_e[2]);
 _photon_frac_reader->AddVariable("L3",&_ml_layer_e[3]);
 _photon_frac_reader->AddVariable("L4",&_ml_layer_e[4]);
 _photon_frac_reader->AddVariable("L5",&_ml_layer_e[5]);
 _photon_frac_reader->AddVariable("L6",&_ml_layer_e[6]);
 _photon_frac_reader->AddVariable("L7",&_ml_layer_e[7]);
 _photon_frac_reader->AddVariable("mpcE3x3",&_ml_mpcE3x3);
 _photon_frac_reader->AddVariable("showerRMS",&_ml_shower_rms);
 _photon_frac_reader->AddVariable("RecoE",&_ml_reco_e);
 _photon_frac_reader->AddVariable("first_layer",&_ml_first_layer);
 _photon_frac_reader->AddVariable("hsx",&_ml_hsx);
 _photon_frac_reader->AddVariable("hsy",&_ml_hsy);
 _photon_frac_reader->AddVariable("Rms0",&_ml_layer_rms[0]);
 _photon_frac_reader->AddVariable("Rms1",&_ml_layer_rms[1]);
 _photon_frac_reader->AddVariable("Rms2",&_ml_layer_rms[2]);
 _photon_frac_reader->AddVariable("Rms3",&_ml_layer_rms[3]);
 _photon_frac_reader->AddVariable("Rms4",&_ml_layer_rms[4]);
 _photon_frac_reader->AddVariable("Rms5",&_ml_layer_rms[5]);
 _photon_frac_reader->AddVariable("Rms6",&_ml_layer_rms[6]);
 _photon_frac_reader->AddVariable("Rms7",&_ml_layer_rms[7]);

 _photon_frac_method = dynamic_cast<TMVA::MethodBase*>(_photon_frac_reader->BookMVA(TMVA::Types::kBDT, (char *)TMVARegression_BDTG_photon_frac_weights_xml)); 

 _pi0_frac_reader = new TMVA::Reader( "!Color:!Silent" );   
 _pi0_frac_reader->AddVariable("L0",&_ml_layer_e[0]);
 _pi0_frac_reader->AddVariable("L1",&_ml_layer_e[1]);
 _pi0_frac_reader->AddVariable("L2",&_ml_layer_e[2]);
 _pi0_frac_reader->AddVariable("L3",&_ml_layer_e[3]);
 _pi0_frac_reader->AddVariable("L4",&_ml_layer_e[4]);
 _pi0_frac_reader->AddVariable("L5",&_ml_layer_e[5]);
 _pi0_frac_reader->AddVariable("L6",&_ml_layer_e[6]);
 _pi0_frac_reader->AddVariable("L7",&_ml_layer_e[7]);
 _pi0_frac_reader->AddVariable("mpcE3x3",&_ml_mpcE3x3);
 _pi0_frac_reader->AddVariable("showerRMS",&_ml_shower_rms);
 _pi0_frac_reader->AddVariable("RecoE",&_ml_reco_e);
 _pi0_frac_reader->AddVariable("first_layer",&_ml_first_layer);
 _pi0_frac_reader->AddVariable("hsx",&_ml_hsx);
 _pi0_frac_reader->AddVariable("hsy",&_ml_hsy);
 _pi0_frac_reader->AddVariable("Rms0",&_ml_layer_rms[0]);
 _pi0_frac_reader->AddVariable("Rms1",&_ml_layer_rms[1]);
 _pi0_frac_reader->AddVariable("Rms2",&_ml_layer_rms[2]);
 _pi0_frac_reader->AddVariable("Rms3",&_ml_layer_rms[3]);
 _pi0_frac_reader->AddVariable("Rms4",&_ml_layer_rms[4]);
 _pi0_frac_reader->AddVariable("Rms5",&_ml_layer_rms[5]);
 _pi0_frac_reader->AddVariable("Rms6",&_ml_layer_rms[6]);
 _pi0_frac_reader->AddVariable("Rms7",&_ml_layer_rms[7]);

 _pi0_frac_method = dynamic_cast<TMVA::MethodBase*>(_pi0_frac_reader->BookMVA(TMVA::Types::kBDT, (char *)TMVARegression_BDTG_pi0_frac_weights_xml)); 

}

double JohnCalibrateShowerEnergy::GetPi0Frac(ExShower* shower){
  _ml_reco_e = shower->GetTotalE();
  _ml_nhits = shower->GetNhits();
  _ml_first_layer = shower->GetFirstLayer();
  _ml_hsx = shower->GetHsx();
  _ml_hsy = shower->GetHsy();

  for(int ilayer=0;ilayer<8;ilayer++){
    _ml_layer_e[ilayer] = shower->GetLayerE(ilayer);
    float t_rms_x = shower->GetLayerHsxRms(ilayer);
    float t_rms_y = shower->GetLayerHsyRms(ilayer);
    float t_rms_r = sqrt(t_rms_x*t_rms_x+t_rms_y*t_rms_y);
    _ml_layer_rms[ilayer] = t_rms_r;
  }
  
  _ml_mpcE3x3 = shower->GetMpcE3x3();
  float t_rms_hsx = shower->GetRmsHsx();
  float t_rms_hsy = shower->GetRmsHsy();

  float t_rms_hsr = sqrt(t_rms_hsx*t_rms_hsx+t_rms_hsy*t_rms_hsy);
  _ml_shower_rms = t_rms_hsr;

  float frac = (_pi0_frac_reader->EvaluateRegression(_pi0_frac_method))[0];

  return frac;
}

double JohnCalibrateShowerEnergy::GetPi0Frac(TMpcExShower* shower){
  _ml_reco_e = shower->get_roughTotE();
  _ml_nhits = shower->sizeHits();
  _ml_first_layer = shower->get_first_layer();
  _ml_hsx = shower->get_hsx();
  _ml_hsy = shower->get_hsy();

  for(int ilayer=0;ilayer<8;ilayer++){
    _ml_layer_e[ilayer] = shower->get_e_layer(ilayer);
    float t_rms_x = shower->get_rms_hsx_layer(ilayer);
    float t_rms_y = shower->get_rms_hsy_layer(ilayer);
    float t_rms_r = sqrt(t_rms_x*t_rms_x+t_rms_y*t_rms_y);
    _ml_layer_rms[ilayer] = t_rms_r;
  }
  
  _ml_mpcE3x3 = shower->get_mpcE3x3();
  float t_rms_hsx = shower->get_rms_hsx();
  float t_rms_hsy = shower->get_rms_hsy();

  float t_rms_hsr = sqrt(t_rms_hsx*t_rms_hsx+t_rms_hsy*t_rms_hsy);
  _ml_shower_rms = t_rms_hsr;

  float frac = (_pi0_frac_reader->EvaluateRegression(_pi0_frac_method))[0];

  return frac;
 
}

double JohnCalibrateShowerEnergy::GetPhotonFrac(ExShower* shower){
  _ml_reco_e = shower->GetTotalE();
  _ml_nhits = shower->GetNhits();
  _ml_first_layer = shower->GetFirstLayer();
  _ml_hsx = shower->GetHsx();
  _ml_hsy = shower->GetHsy();

  for(int ilayer=0;ilayer<8;ilayer++){
    _ml_layer_e[ilayer] = shower->GetLayerE(ilayer);
    float t_rms_x = shower->GetLayerHsxRms(ilayer);
    float t_rms_y = shower->GetLayerHsyRms(ilayer);
    float t_rms_r = sqrt(t_rms_x*t_rms_x+t_rms_y*t_rms_y);
    _ml_layer_rms[ilayer] = t_rms_r;
  }
  
  _ml_mpcE3x3 = shower->GetMpcE3x3();
  float t_rms_hsx = shower->GetRmsHsx();
  float t_rms_hsy = shower->GetRmsHsy();

  float t_rms_hsr = sqrt(t_rms_hsx*t_rms_hsx+t_rms_hsy*t_rms_hsy);
  _ml_shower_rms = t_rms_hsr;

  float frac = (_photon_frac_reader->EvaluateRegression(_photon_frac_method))[0];

  return frac;
 
}

double JohnCalibrateShowerEnergy::GetPhotonFrac(TMpcExShower* shower){
  _ml_reco_e = shower->get_roughTotE();
  _ml_nhits = shower->sizeHits();
  _ml_first_layer = shower->get_first_layer();
  _ml_hsx = shower->get_hsx();
  _ml_hsy = shower->get_hsy();

  for(int ilayer=0;ilayer<8;ilayer++){
    _ml_layer_e[ilayer] = shower->get_e_layer(ilayer);
    float t_rms_x = shower->get_rms_hsx_layer(ilayer);
    float t_rms_y = shower->get_rms_hsy_layer(ilayer);
    float t_rms_r = sqrt(t_rms_x*t_rms_x+t_rms_y*t_rms_y);
    _ml_layer_rms[ilayer] = t_rms_r;
  }
  
  _ml_mpcE3x3 = shower->get_mpcE3x3();
  float t_rms_hsx = shower->get_rms_hsx();
  float t_rms_hsy = shower->get_rms_hsy();

  float t_rms_hsr = sqrt(t_rms_hsx*t_rms_hsx+t_rms_hsy*t_rms_hsy);
  _ml_shower_rms = t_rms_hsr;

  float frac = (_photon_frac_reader->EvaluateRegression(_photon_frac_method))[0];

  return frac;
}

int JohnCalibrateShowerEnergy::isInnerTower(int x, int y, int arm){

  int retVal = 0;

  if(arm == 0){
    
    if( x==4 && ( (y==6) || (y==7) || (y==8) || (y==9) || (y==10) || (y==11)) ){//left edge
      retVal = 1;
    }
    else if( (x==5) && ( (y==5) || (y==12) ) ){//left corners(top and bottom)
      retVal = 1;
    }
    else if( y==13  && ( (x==6) || (x==7) || (x==8) || (x==9) || (x==10) || (x==11) ) ){//top row
      retVal = 1;
    }
    else if( y==4  && ( (x==6) || (x==7) || (x==8) || (x==9) || (x==10) || (x==11) ) ){//bottom row
      retVal = 1;
    }
    else if( x==12 && ( (y==5) || (y==12) ) ){//right corners (top and bottom)
      retVal = 1;
    }
    else if( x==13  && ( (y==6) || (y==7) || (y==8) || (y==9) || (y==10) || (y==11) ) ){//right edge
      retVal = 1;
    }
    else{//not inner tower
      retVal = 0;
    }

  }//end arm 0

  if(arm == 1){

    if( x==5 && ( (y==7) || (y==8) || (y==9) || (y==10) ) ){//left edge
      retVal = 1;
    }
    else if( x==6 && ( (y==6) || (y==11) ) ){//left corners(top and bottom)
      retVal = 1;
    }
    else if( y==12  && ( (x==7) || (x==8) || (x==9) || (x==10) ) ){//top row
      retVal = 1;
    }
    else if( y==5  && ( (x==7) || (x==8) || (x==9) || (x==10) ) ){//bottom row
      retVal = 1;
    }
    else if( x==11 && ( (y==6) || (y==11) ) ){//right corners (top and bottom)
      retVal = 1;
    }
    else if( x==12 && ( (y==7) || (y==8) || (y==9) || (y==10) ) ){//right edge
      retVal = 1;
    }
    else{
      retVal = 0;
    }
  }//end arm 1

  return retVal;

}//end isInnerTower()


int JohnCalibrateShowerEnergy::isOuterTower(int x, int y, int arm){

  int retVal = 0;

  if(arm == 0){

    if( ( (x==0)||(x==17)) && ( (y==6) || (y==7) || (y==8) || (y==9) || (y==10) || (y==11) ) ){
      retVal = 1;
    }
    else if( ((x==1)||(x==16)) && ( (y==4) || (y==5) || (y==12) || (y==13) ) ){
      retVal = 1;
    }
    else if( (((x==2)||(x==15)) && ((y==3)||(y==14))) ){
      retVal = 1;
    }
    else if( (((x==3)||(x==14)) && ((y==2)||(y==15))) ){
      retVal = 1;
    }
    else if( ((y==1) || (y==16)) && ((x==4) || (x==5) || (x==12) || (x==13)) ){
      retVal = 1;
    }
    else if( ((y==0) || (y==17)) && ((x==6) || (x==7) || (x==8) || (x==9) || (x==10) || (x==11)) ){
      retVal = 1;
    }
    else{
      retVal = 0;
    }
  }//end arm 0

  if(arm == 1){

    if( ( (x==0)||(x==17)) && ( (y==7) || (y==8) || (y==9) || (y==10) ) ){
      retVal = 1;
    }
    else if( ((x==1)||(x==16)) && ( (y==4) || (y==5) || (y==6) || (y==11) || (y==12) || (y==13) ) ){
      retVal = 1;
    }
    else if( (((x==2)||(x==15)) && ((y==3)||(y==14))) ){
      retVal = 1;
    }
    else if( (((x==3)||(x==14)) && ((y==2)||(y==15))) ){
      retVal = 1;
    }
    else if( ((y==1) || (y==16)) && ((x==4) || (x==5) || (x==12) || (x==13)) ){
      retVal = 1;
    }
    else if( ((y==0) || (y==17)) && ((x==6) || (x==7) || (x==8) || (x==9) || (x==10) || (x==11)) ){
      retVal = 1;
    }
    else{
      retVal = 0;
    }


  }//end arm 1

  return retVal;

}//end isOuterTower
