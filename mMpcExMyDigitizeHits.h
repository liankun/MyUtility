#ifndef __MMPCEXMYDIGITIZEHITS_H__
#define __MMPCEXMYDIGITIZEHITS_H__

/**
 * @class  mMpcExMyDigitizeHits
 * @author ngrau@augie.edu
 * @date   July 2015
 * @brief  Uses the measured minipad-by-minipad pedestals and Gaussian widths to create a realistic digitized signal including noise from an input set of GEANT minipad energies. The incoming GEANT hit enerties are digitized assuming 150 keV for most probable landau energy loss.
 */

//full simulate the zero suppress for MpcEx
//MpcEx uses two minpad to surpress
//the surppress map from Milap:
// I know it for the online configuration when making the threshold
// files for each packet, i.e 21301-8, 21351-8.  Each channel (0 -
// 12*128) in the even chains and odd chains are linked, meaning that
// channels in chains 0&2 and 1&3 are both written as one.  So if the
// channel in chain 1 passes the threshold, you'll get information about
// that channel in chain 3.

#include <map>
#include <SubsysReco.h>
#include <MpcExConstants.h>
class PHCompositeNode;
class TRandom3;
class TFile; 
class TH2D; 
class TMpcExHit;
class MpcExRawHit;

class mMpcExMyDigitizeHits : public SubsysReco {

 public:

  //! constructor -- with the number of below above the 
  //! pedestal to suppress, which emulates the DCM zero
  //! suppression
  mMpcExMyDigitizeHits();

  //! destructor
  virtual ~mMpcExMyDigitizeHits();

  //! Applies known pedestal, width, and mip to each
  //! channel to digitize PISA energy into an ADC count
  int process_event(PHCompositeNode *topNode);

  int Init         (PHCompositeNode *topNode);
  int End          (PHCompositeNode *topNode);

  void perform_DCM_suppress(std::map<unsigned int,TMpcExHit*>& hit_list,MpcExRawHit *rawhits);
  
  //the default value will be very small
  //to reduce the smear effect
  //in percentage
  void set_energy_smear_width(double width = 0.000001){_energy_smear_width = width;}

  enum SuppressMode {
    FIX_SUP = 0, // assuem each minipad is suppressed,use default suppress
    FIX_SUP_REAL = 1, // dual minipads is suppressed
    DEFAULT_CUT_SUP = 2,
    DEFAULT_CUT_SUP_REAL = 3,
    FIT_MPV_CUT_SUP = 4,
    FIT_MPV_CUT_SUP_REAL = 5
  };

  enum SmearMode { 
    SAME_SMEAR = 0, //same smear factor for all minipads
    DIFF_SMEAR =1 //each minipads has its own smear factor
  };

  void set_suppress_mod(SuppressMode mod){_sup_mod = mod;}
  void set_smear_mod(SmearMode mod){_smear_mod = mod;}

 private:

  //! Calculates the integer digitized channel for an input 
  //! energy including the pedestal mean position and width 
  //! and the mip position assuming the mip is 150 keV.
  float make_adc(float mip, float mean, float width, float energy, float *esat);

  //! random number generator
  TRandom3 *_dice;

  int makeHisto;
  TFile *outputfile; 
  TH2D *_histo_low; 
  TH2D *_histo_high; 
  TH2D *_histo_low_dcm; 
  TH2D *_histo_high_dcm; 

  SuppressMode _sup_mod;
  SmearMode _smear_mod;
  
  double _energy_smear_width; //energy smear width in percentage
  

  float _default_cut_high_adc[MpcExConstants::NMINIPADS];
  float _fit_mpv_cut_high_adc[MpcExConstants::NMINIPADS];
  float _default_cut_low_adc[MpcExConstants::NMINIPADS];
  float _fit_mpv_cut_low_adc[MpcExConstants::NMINIPADS];
  float _e_smear[MpcExConstants::NMINIPADS];


};

#endif /* __MMPCEXDIGITIZEHITS_H__ */
