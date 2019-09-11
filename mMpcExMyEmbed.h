// Note : 
// 1. Please set same SigNodeName, BgNodeName as you set in your macro
// 2. Make sure when you print vertex from real DST don't skip event (like |BBCZ| > 30) it can cause sync issue
// 3. I recommend to 0 Vertex smearing in pisaToDST.C so we can compare vertex without err
// 4. Set proper starting point in your macro (use Fun4AllInputManager::skip(...))
// 5. Don't call mMpcExDigitizeHits in your macro

// Oct. 17th 2016 -Jh.Do


#ifndef __MMPCEXMYEMBED_H__
#define __MMPCEXMYEMBED_H__

#include "SubsysReco.h"
#include "TFile.h"
#include "TMpcExHitContainer.h"
#include "PHGlobal.h"
#include "TMpcExCalibContainer.h"
#include "MpcExRawHit.h"
#include "TMpcExGeaHitContainer.h"
#include "Fun4AllServer.h"
#include "TH1D.h"
#include "TH2F.h"
#include <stdlib.h>
#include "mMpcExDigitizeHits.h"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include "TRandom3.h"
#include "Exogram.h"
#include "mMpcExApplyCalibrations.h"


class mMpcExMyEmbed : public SubsysReco {

 public:

  //! constructor
  mMpcExMyEmbed();
  //! destructor
  virtual ~mMpcExMyEmbed();

  //! create the in-memory nodes on the node tree
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode* topNode);
  int EventCheck(PHCompositeNode *topNode); // Vertex matching check
  int EmbedMpcEx();
  int FindHit(unsigned int key_seek, std::vector<unsigned int> &raw);
  int GetNode(PHCompositeNode* topNode);
  //////
  int ZeroSup(int ADC, int Type, int arm, double ped_mean, double ped); //Type 0 : High, 1 : Low
  void DebugMode(int v) { fDebug = v; }
  void QAFlag(int v) { fQAFlag = v; }
  void SetSigNodeName(std::string name) {fSigNodeName = name;}
  void SetBgNodeName(std::string name) {fBgNodeName = name;}
  void SetVertexDiffMax(double val) {fVertexDiff = val;}

  //QA Histogram
  TH1D *hVertexDiff;
  TH1D *hMpcExDiff;
  TH1D *hMpcExNHit;
  TH2F *hMpcExAdcHL[3]; // 0:Sig, 1:Bg, 2:Embed
  TH2F *hEmbedHighADC;
  TH2F *hEmbedLowADC;
  TH2D *hEventDis[3][5]; // 0:Sig, 1:Bg, 2:Embed | Event Index

  Exogram* exo_Sig;
  Exogram* exo_Bg;
  Exogram* exo_Em;

 private:
  std::string fSigNodeName;
  std::string fBgNodeName;
  TMpcExGeaHitContainer *fMpcExGeaHitCont;
  TRandom3 *fDice;
  MpcExRawHit *fMpcExRawHit_Bg;
  PHGlobal *fPHGlobal_Sig;
  PHGlobal *fPHGlobal_Bg;
  TMpcExCalibContainer *fMpcExCalibCont;
  MpcExMapper *mpcexmap;
  int fDebug;
  int fQAFlag;
  int IEvt;
  int fNErr;
  double fVertexDiff;

  //! Disable the layer MPV adjustment
  int disable_MPV_layer_adjust; 

  mMpcExApplyCalibrations::Mode calibMode;

};


#endif /*__MMPCEXEMBED_H__*/
