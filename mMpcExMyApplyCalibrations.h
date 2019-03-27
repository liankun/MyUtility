#ifndef __MMPCEXMYAPPLYCALIBRATIONS_H__
#define __MMPCEXMYAPPLYCALIBRATIONS_H__

/**
 * @class  mMpcExMyApplyCalibrations
 * @author ngrau@augie.edu 
 * @date   July 2015
 * @brief  Apply the minipad-by-minipad calibrations
 */

#include <SubsysReco.h>
#include <vector>
#include "MpcExConstants.h"
#include "TMpcExHit.h"
class PHCompositeNode;
class MpcExEventHeader;
class TRandom3;
class TFile;
class TH2D;
class TH1D;
class Exogram;
class TMpcExCalib; 

class mMpcExMyApplyCalibrations : public SubsysReco {

 public:

  enum Mode {
    PEDESTAL_SUBTRACTED_ONLY = 0,
    PEDESTAL_AND_CMN_SUBTRACTED_ONLY = 1,
    COMPLETE_FIXED_MC_PERFECT = 2, // FIXED CALIBS, PERFECT DETECTOR
    COMPLETE_FIXED_MC = 3,         // FIXED CALIBS, USE DB FOR DEAD-HOT
    COMPLETE_FIXED_REALPED = 4,    // FIXED CALIBS, BUT USE PEDESTAL AND DEAD-HOT
    COMPLETE = 5,                  // FOR REAL DATA WITH ALL CALIBRATIONS
  }; 

  //! Constructor
  mMpcExMyApplyCalibrations();
  
  void set_50kev_cutoff(){_50kev_cut_off = true;  }

  //! Constructor
  virtual ~mMpcExMyApplyCalibrations();

  //! Apply the minipad-by-minipad calibrations and fill a TMpcExCalibHitContainer
  int process_event(PHCompositeNode *topNode);

  int Init         (PHCompositeNode *topNode);
  int InitRun      (PHCompositeNode *topNode);
  int End          (PHCompositeNode *topNode);

 private:

  //! Check Cell ID's for mismatches/bad values
  int CellIDCheck(MpcExEventHeader *evt_head);

  //! Check PARST times for mismatches/bad values
  int PARSTCheck(MpcExEventHeader *evt_head);

  //! Cuts on Statphase for Stack>1
  int StatePhaseCheck(MpcExEventHeader *evt_head);

  //! Common mode noise subtraction
  void ApplyCommonModeNoiseSubtraction();

  //! Sigma cut on hits
  void SigmaCut(TMpcExHit *hit, TMpcExCalib *calib); 
  void SigmaCutMIP(TMpcExHit *hit, TMpcExCalib *calib); 

  //! internal array that determines if a chip locks up
  unsigned int _FailCellIDCheck[MpcExConstants::NARMS][MpcExConstants::NPACKETS_PER_ARM][MpcExConstants::NCHAINS_PER_PACKET][MpcExConstants::NCHIPS_PER_CHAIN];

  //! internal array for temporary storage of hits for CMN subtraction
  std::vector<TMpcExHit*> hit_list[MpcExConstants::NARMS][MpcExConstants::NPACKETS_PER_ARM][MpcExConstants::NCHAINS_PER_PACKET][MpcExConstants::NCHIPS_PER_CHAIN];

  //! flag check for CMN subtraction
  int doCMNsubtract; 

  //! flag check for ADC sigma cut
  int doSigmaCut; 

  //! flag check for ADC sigma cut (MIP analysis)
  int doSigmaCutMIP; 

  //! flag check for fixed calibrations
  Mode calibMode;

  //! eliminate bad hits when calibrating
  int eliminateBad; 

  //! flag to make histograms, set when the string in RecoConsts "MpcExApplyCalibHistoFileName" is not empty
  int _makeHisto;

  //! flag to specify the arm of the hits that will be kept (only one arm and packet at a time), set as an integer in RecoConsts "MpcExApplyCalibHistoArm"
  int _histo_arm;

  //! flag to specify the packet of the hits that will be kept (only one arm and packet at a time), set as an integer in RecoConsts "MpcExApplyCalibHistoPacket"
  int _histo_packet;

  //! apply stack=1 cut
  int applyStackCut;

  //! H/L Ratio consistency cut
  int H_L_ConsistencyCut; 

  //! Disable the layer MPV adjustment
  int disable_MPV_layer_adjust; 

  TFile *_outputfile;
  TH2D *_histo_pedsub_highlow[3072];
  TH2D *_histo_raw_low;
  TH2D *_histo_raw_high;
  TH2D *_histo_pedsub_low;
  TH2D *_histo_pedsub_high;
  TH2D *_histo_sensorCalib_low;
  TH2D *_histo_sensorCalib_high;
  TH2D *_histo_fullCalib_low;
  TH2D *_histo_fullCalib_high;
  TH1D *_histo_badcellid;
  TH2D *_histo_low;
  TH2D *_histo_high;
  TH2D *_histo_combined;
  TH1D *_histo_numHits_S;
  TH1D *_histo_numHits_N;
  TH2D *_histo_raw_low_parst;
  TH2D *_histo_raw_high_parst;
  TH2D *_histo_pedsub_low_parst;
  TH2D *_histo_pedsub_high_parst; 
  TH2D *_histo_ped_low;
  TH2D *_histo_ped_high;
  TH2D *_histo_pedsigma_low;
  TH2D *_histo_pedsigma_high;
  TH2D *_histo_pedchi2_low;
  TH2D *_histo_pedchi2_high;
  TH2D *_histo_pedsub_low_statephase;
  TH2D *_histo_pedsub_high_statephase; 
  Exogram *_histo_low_hotdead[2];
  Exogram *_histo_high_hotdead[2];

  // random number generator
  TRandom3 *r3; 
  float _e_fit_cor[49152];
  int _good_low_gain_neighbor[49152];

  bool _50kev_cut_off;


};

#endif /* __MMPCEXCALIBRATEHITS_H__ */
