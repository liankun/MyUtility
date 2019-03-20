#ifndef __MMPCEXMYLOADCALIBRATIONS_H__
#define __MMPCEXMYLOADCALIBRATIONS_H__

/**
 * @class  mMpcExLoadCalibration 
 * @author ngrau@augie.edu
 * @date   July 2015
 * @brief  This SubsysReco reads calibration data and fills the in-memory TMpcExCalibContainer
 */

#include <SubsysReco.h>
class PHCompositeNode;
class TMpcExCalibContainer;

class mMpcExMyLoadCalibrations : public SubsysReco {

 public:

  //! Constructor
  mMpcExMyLoadCalibrations();

  //! Destructor
  virtual ~mMpcExMyLoadCalibrations();

  //! Read the calibration data for a given run 
  int InitRun(PHCompositeNode *topNode);

  //! Because the calibrations persist through all events in a run, clear them at the end
  int EndRun(PHCompositeNode *topNode);

};

#endif /* __MMPCEXMYLOADCALIBRATIONS_H__ */
