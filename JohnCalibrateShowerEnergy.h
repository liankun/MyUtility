#ifndef __JOHNCALIBRATESHOWERENERGY_H__
#define __JOHNCALIBRATESHOWERENERGY_H__

#include "TMVA/Types.h"
#include "TMVA/Reader.h"
#include "TMVA/IMethod.h"

class ExShower;

class MpcExTSpline1; 
class TString; 

class JohnCalibrateShowerEnergy {
  
 private:


 public:

  //! destructor
  ~JohnCalibrateShowerEnergy();

  static JohnCalibrateShowerEnergy* instance();
 
  void CalibrateEnergy(TMpcExShower *shower_v1,  bool use_cutoff = true );
  void CalibrateEnergyDecoupledPair(TMpcExShower *shower_v1, int nShared);
  float RecalCalibratedEnergy(TMpcExShower *shower_v1, float MpcEx_E, float MPC_E, 
			      float *MPCXE, float *ALPLTE, float *MPCE, int *CalOK, bool pairFlag);
  float DoCalibrateEnergy(TMpcExShower *shower_v1, float MpcEx_E, float MPC_E, 
			  float *MPCXE, float *ALPLTE, float *MPCE, bool setShower, int *CalOK, bool pairFlag = false, int nShared = 0);
  
  double DoCalibrateEnergy(int arm,double MpcEx_E,double MPC_E,double vtx,
                          double hsx,double hsy,int pk_ix,int pk_iy,
			  int first_layer,int MpcN3x3,int n_sat_mpads);
  double CalibrateUseMpcEx(double layer_e[8],double shower_rms,int nhits,
                           int first_layer,int nsat,double hsx,double hsy);

  int isOuterTower(int x, int y, int arm);
  int isInnerTower(int x, int y, int arm);

  double GetPhotonFrac(ExShower* shower);
  double GetPhotonFrac(TMpcExShower* shower);
  double GetPi0Frac(ExShower* shower);
  double GetPi0Frac(TMpcExShower* shower);

 private:

  //!constructor
  JohnCalibrateShowerEnergy();

  //! the single instance of this class
  static JohnCalibrateShowerEnergy* _instance;

  void SetSplines(); 
  void SetupRegression(); 

  MpcExTSpline1 *ex_calSpline[2][5]; 
  MpcExTSpline1 *mpc_calSpline[2][4]; 
  MpcExTSpline1 *al_calSpline[2]; 
  MpcExTSpline1 *comb_calSpline[2][4]; 

  float comblcor[2][5]; 

  // Energy weighting constants from minimization
  // JGL 11/13/2018 determined by Minuit minimization of linearty w.r.t straight line 

  double CNST[2][4][4];  

  // Total energy from Machine Learning methods
  // TMVA readers: 

  TMVA::Reader *reader[2][3];
  TMVA::Reader *reader_E33_0[2];

  //MpcEx only calibration
  TMVA::Reader *_mpcex_only_reader;
  TMVA::Reader *_pi0_frac_reader;
  TMVA::Reader *_photon_frac_reader;

  TMVA::MethodBase *method[2][3]; 
  TMVA::MethodBase *method_E33_0[2]; 
  TMVA::MethodBase *_mpcex_only_method;
  TMVA::MethodBase *_photon_frac_method;
  TMVA::MethodBase *_pi0_frac_method;

  TMVA::Types::EMVA methodType; 

  float ml_resum, ml_Cangle, ml_E33, ml_in33, ml_insat, ml_ifl, ml_ffx, ml_ffy, ml_vtx; 

  float _ml_layer_e[8];
  float _ml_layer_rms[8];
  float _ml_mpcE3x3;
  float _ml_reco_e;
  float _ml_shower_rms;
  float _ml_nhits;
  float _ml_first_layer;
  float _ml_nsat;
  float _ml_hsx;
  float _ml_hsy;
 
};


#endif /* __MPCEXCALIBRATESHOWERENERGY_H__*/

