#ifndef __MMPCEXSHOWERV2_HH__
#define __MMPCEXSHOWERV2_HH__

#include <memory>
#include <vector>
#include <map>
#include <TLorentzVector.h>
#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include "MpcExConstants.h"
#include <iostream>

class SubsysReco;
class PHCompositeNode;
class MpcMap;
class mpcClusterContainer;
class mpcTowerContainer;
class mpcRawContainer;
class TMpcExHitContainer;
class TMpcExShowerContainer; 
class TMpcExShower; 
class MpcExMapper; 
class TMpcExHit; 
class MpcExCalibrateShowerEnergy; 

// Shower algorithm parameters
#define MAX_ITERATIONS 100
#define MAX_SUBSHOWER_ITERATIONS MAX_ITERATIONS
#define HOUGH_CONVERGE 0.00025  

// MPC-EX shower cone size (cm) 
#define SHOWER_BACK_RADIUS 1.60 
#define SHOWER_FRONT_RADIUS 0.80
#define SHOWER_RADIUS SHOWER_BACK_RADIUS

// Second stage shower expansion limits
#define EXPANSION_START 1.3
#define FRACTIONAL_ENERGY_LIMIT 0.02
#define EXP_FACTOR_LIMIT 4.0

class mMpcExShowerV2: public SubsysReco
{
 public:

  mMpcExShowerV2( const char* name = "MMPCEXSHOWERV2" );
  virtual int Init(PHCompositeNode*);
  virtual int InitRun(PHCompositeNode*);
  virtual int process_event(PHCompositeNode*);
  virtual ~mMpcExShowerV2();
  virtual int End(PHCompositeNode *topNode);

  void EnableSTReco() {DoSTReco = 1; std::cout << "mMpcExShowerV2: single track reconstruction enabled" << std::endl;}
  void DisableSTReco() {DoSTReco = 0; std::cout << "mMpcExShowerV2: single track reconstruction disabled" << std::endl;}

  void EnableSouthReco() {DisableSouth = 0; std::cout << "mMpcExShowerV2: south arm reconstruction enabled" << std::endl;}
  void DisableSouthReco() {DisableSouth = 1; std::cout << "mMpcExShowerV2: south arm reconstruction disabled" << std::endl;}

  void EnableNorthReco() {DisableNorth = 0; std::cout << "mMpcExShowerV2: north arm reconstruction enabled" << std::endl;}
  void DisableNorthReco() {DisableNorth = 1; std::cout << "mMpcExShowerV2: north arm reconstruction disabled" << std::endl;}

 private:

  void set_interface_ptrs(PHCompositeNode* top_node);
  bool FindShowers();
  void ClusterDeadAreas(); 
  void SingleTrackReco(); 
  void MergeShowers(TMpcExShower *main, TMpcExShower *splinter);
  void CalculateShowerProperties(TMpcExShower *shower_v1);

  void getMPC_TwrSum(int arm, double shx, double shy, int &iMPCTwr_peak, double MPCE[5][5], 
		     double MPCTOF[5][5], double MPCHS[5][5][2], double MPCQUAL[5][5],
		     int &N_3x3, int &N_5x5, int &fiducial, int &pkix, int &pkiy ); 
  void MatchMPC(int arm, double fMPCEXHoughX, double fMPCEXHoughY, float &fMPC_dhough, 
		float &distx, float &disty, float &clustE, int &clustNum);

  double CombinedHitEnergy(TMpcExHit* hit_ptr);

  TMpcExHitContainer* _hit_map;
  TMpcExShowerContainer* _shower_map; 
  mpcClusterContainer *_mpc_cluster_container;
  mpcTowerContainer *_mpc_tower_container;
  mpcRawContainer *_mpcraw2_container;
  MpcMap *_mpc_map;
  MpcExMapper *_mapper; 

  MpcExCalibrateShowerEnergy* _CalEnergy; 

  int fLayerValid[MpcExConstants::NLAYERS];

  protected:

  double _vertex;
  double _t0; 

  int DoSTReco; 
  int DisableSouth; 
  int DisableNorth; 
 
};

#endif /* __MMPCEXSHOWER_H__ */ 
