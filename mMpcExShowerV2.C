#include "mMpcExShowerV2.h"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "TMpcExHitSet.h"
#include "TMpcExShowerContainer.h"
#include "TMpcExShower.h"
#include "MpcExCalibrateShowerEnergy.h"

#include <PHIODataNode.h>
#include <getClass.h>
#include <BbcOut.h>
#include <PHGlobal.h>
#include <Fun4AllReturnCodes.h>
#include <recoConsts.h>
#include <primaryWrapper.h>

//MPC Containers
#include <MpcMap.h> 
#include <mpcClusterContent.h>
#include <mpcClusterContainer.h>
#include <mpcClusterContentV1.h>
#include <mpcTowerContainer.h>
#include <mpcTowerContent.h>
#include <mpcTowerContentV1.h>
#include <mpcRawContainer.h>
#include <mpcRawContent.h>

// STL/BOOST
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// ROOT
#include <TLorentzVector.h>
#include <TVector3.h>

// Second stage shower expansion limits
#define EXPANSION_START 1.3
#define FRACTIONAL_ENERGY_LIMIT 0.02
#define EXP_FACTOR_LIMIT 4.0


using namespace std;
using namespace findNode;
typedef PHIODataNode<PHObject> PHObjectNode_t; 

struct OrderByKey : public std::binary_function<TMpcExHit*,TMpcExHit*,bool> {
public:
  bool operator()(TMpcExHit* lhs, TMpcExHit* rhs){
    return lhs->key() < rhs->key();
  }
};

// Order the arm=0 hits first, followed by arm1
// Within the arm order the hits with the largest energy first 

struct OrderByEnergy : public std::binary_function<TMpcExHit*,TMpcExHit*,bool> {
public:
  bool operator()(TMpcExHit* lhs, TMpcExHit* rhs){
	  
    double lhs_newkey = lhs->arm()*100000 + (255*(147.6/19.0)*4.5 - lhs->combined()); 
    double rhs_newkey = rhs->arm()*100000 + (255*(147.6/19.0)*4.5 - rhs->combined()); 

    return lhs_newkey <= rhs_newkey;
  }
};

struct UnionElement{
  int prev;
  int index;
  int num;

  void init(int i){
    prev = i;
    index = i;
    num = 1;
  }
};


double ex_size = 20;
double ex_z = 203.982;
double box_size = 0.2;
UnionElement ele_array[49152];
const int nbox = (int(ex_size/box_size)+1)*2;


int get_root(int index){
  int root = index;
  while(root!=ele_array[root].prev) root=ele_array[root].prev;
  return root;

}

void merge_UnionElement(int index1,int index2){
  if(index1<0 || index1>=49152){
    cout<<"bad index1 "<<index1<<endl;
    return;
  }

  if(index2<0 || index2>=49152){
    cout<<"bad index2 "<<index2<<endl;
    return;
  }

  int root1 = get_root(index1);
  int root2 = get_root(index2);

  if(ele_array[root1].num>ele_array[root2].num){
    ele_array[root2].prev = root1;
    ele_array[root1].num += ele_array[root2].num;
  
  }
  else{
    ele_array[root1].prev = root2;
    ele_array[root2].num += ele_array[root1].num;
  }
}



//_______________________________________________________
mMpcExShowerV2::mMpcExShowerV2( const char* name ) : 
  SubsysReco( name )
{
  _vertex = -9999.0;
  _t0 = -9999.0; 
  
  //Define which x-y layers are valid for analysis
  recoConsts *myrc = recoConsts::instance();
  int fPattern = myrc->get_IntFlag("mpcexUseLayers",0x11111111); 
  
  for(int i=0 ; i<MpcExConstants::NLAYERS ; i++) {
    fLayerValid[i] = ((fPattern&int(pow(16,i)))==pow(16,i) ? 1:0);
    //cout << "fLayerValid[" << i << "] = " << fLayerValid[i] << endl; 
  }
  DoSTReco = 0; // disabled by default
  DisableSouth = 0; 
  DisableNorth = 0; 

  //set all other private members
  _mapper = NULL; 
  _hit_map = NULL;
  _shower_map = NULL;
  _mpc_cluster_container = NULL;
  _mpc_tower_container = NULL;
  _mpc_map = NULL;
  _mpcraw2_container= NULL;

  _CalEnergy = MpcExCalibrateShowerEnergy::instance(); 

}

//_______________________________________________________
// destructor
int mMpcExShowerV2::End(PHCompositeNode *topNode)
{
  return EVENT_OK;
}

//_______________________________________________________
// destructor
mMpcExShowerV2::~mMpcExShowerV2()
{
 
 
}

//_______________________________________________________
// initialization
int mMpcExShowerV2::Init(PHCompositeNode* top_node)
{

  //add the shower node 
  PHNodeIterator nodeIter(top_node);
  PHCompositeNode *dstNode = static_cast<PHCompositeNode*>(nodeIter.findFirst("PHCompositeNode","DST"));

  _shower_map = new TMpcExShowerContainer();
  PHIODataNode<TMpcExShowerContainer>* showerNode = new PHIODataNode<TMpcExShowerContainer>(_shower_map,"TMpcExRawShowerContainer","PHObject");
  dstNode->addNode(showerNode);

  // get an instance of the mapper
  _mapper = MpcExMapper::instance(); 

  return EVENT_OK; 

}

int mMpcExShowerV2::InitRun(PHCompositeNode* top_node)
{
 
  // Reset IOC pointers
  set_interface_ptrs(top_node);
  
  // clear maps
  _shower_map->Reset(); 

  return EVENT_OK;
}


//_______________________________________________________
/*! Reset IOC and external interface pointers */
void mMpcExShowerV2::set_interface_ptrs(PHCompositeNode* top_node)
{
  // mpcex hit map pointer
  _hit_map = getClass<TMpcExHitContainer>(top_node, "TMpcExHitContainer");
  if(!_hit_map) { cout << PHWHERE << ":: No TMpcExHitContainer No sense continuing" << endl; exit(1);}

  //MPC objects in the dst
  _mpc_map = MpcMap::instance();
  if(!_mpc_map){ cout << "No MpcMap! No sense continuing" << endl; exit(1);} 

  _mpc_tower_container = getClass<mpcTowerContainer> (top_node, "mpcTowerContainer");
  if(!_mpc_tower_container) { cout << "No mpcTowerContainer! No sense continuing" << endl; exit(1);}

  _mpc_cluster_container = getClass<mpcClusterContainer> (top_node, "mpcClusterContainer");
  //if(!_mpc_cluster_container) { cout << PHWHERE << ":: No mpcClusterContainer! No sense continuing" << endl; exit(1);}
  if(!_mpc_cluster_container) { cout << PHWHERE << ":: No mpcClusterContainer! " << endl;}

  _mpcraw2_container = findNode::getClass<mpcRawContainer>(top_node,"MpcRaw2");
  if(!_mpcraw2_container) { cout << PHWHERE << ":: No MpcRaw2 container! " << endl;}

}

//_______________________________________________________
// Event method.
int mMpcExShowerV2::process_event(PHCompositeNode* top_node)
{

  PHGlobal *phglobal                     = getClass<PHGlobal>                (top_node, "PHGlobal");
  BbcOut *bbcout                         = getClass<BbcOut>                  (top_node, "BbcOut");

  if(!bbcout && !phglobal){ 
    //cout << "No BbcOut or PHGlobal!  No sense continuing" << endl; 
    //exit(1);
    _vertex = 0.0;
    _t0 = 0.0; 
  }
  else{
    _vertex=(phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
    _t0 = (phglobal==0) ? phglobal->getBbcTimeZero() : bbcout->get_TimeZero();
  }
  
  _shower_map->Reset(); 
  
  FindShowersV2(top_node);
  
//  std::cout<<"Number of hits: "<<_hit_map->size()<<std::endl;
//  std::cout<<"Number showers: "<<_shower_map->size()<<std::endl;
  if(DoSTReco)
    SingleTrackReco(); 

  return EVENT_OK;
}

bool  mMpcExShowerV2::FindShowers(PHCompositeNode* top_node)
{
       
  //static int numEvt = 0;  
  //if( (numEvt%1) == 0) std::cout << "showering event = " << numEvt 
  //				 << " (" << _hit_map->sHits() << "," << _hit_map->nHits() << "}" << std::endl; 
  //numEvt++; 

  vector<unsigned int> used_list; 
  vector<unsigned int> current_used_list; 
  
  used_list.clear();

  TMpcExHitSet<OrderByEnergy> hit_set(_hit_map);
  TMpcExHitSet<OrderByEnergy>::const_iterator itr_arm0 = hit_set.get_iterator();
  TMpcExHitSet<OrderByEnergy>::const_iterator last_arm1 = hit_set.end();
  TMpcExHitSet<OrderByEnergy>::const_iterator itr_arm1 = hit_set.get_iterator();
  std::advance(itr_arm1,_hit_map->sHits()); 
  TMpcExHitSet<OrderByEnergy>::const_iterator last_arm0 = hit_set.get_iterator();
  std::advance(last_arm0,_hit_map->sHits()); 

  // Set the cluster radii based on the layer, event vertex
  float shower_radius[MpcExConstants::NARMS][MpcExConstants::NLAYERS]; // radius in hough space
  for(int i=0;i<MpcExConstants::NLAYERS;i++){

    float shower_rad_layer = SHOWER_FRONT_RADIUS + 
      ((SHOWER_BACK_RADIUS-SHOWER_FRONT_RADIUS)/(MpcExConstants::NLAYERS-1))*i;

    shower_radius[0][i] = fabs(shower_rad_layer/(MpcExConstants::PS_REFERENCE_Z_S -_vertex)); 
    shower_radius[1][i] = fabs(shower_rad_layer/(MpcExConstants::PS_REFERENCE_Z_N -_vertex)); 
  }

  // The main shower-finding section
  TMpcExHitSet<OrderByEnergy>::const_iterator itr;
  TMpcExHitSet<OrderByEnergy>::const_iterator itr_end;

  if(DisableSouth)
    itr = itr_arm1; 
  else
    itr = itr_arm0;

  if(DisableNorth)
    itr_end = last_arm0; 
  else
    itr_end = last_arm1;
 
  for(; itr!=itr_end; ++itr){
    TMpcExHit* hit_ptr = *itr;

    // only good energy calibrated hits
    if(!hit_ptr->isGoodCombinedHit()){ 
      used_list.push_back(hit_ptr->key());
      continue;
    }

    // Only use *active* strip layers (user defined)
    short layer = hit_ptr->layer();
    if(fLayerValid[layer] == 0) { 
      used_list.push_back(hit_ptr->key());
      continue; 
    } 

    // make sure this hit isn't already used
    if(std::find(used_list.begin(), used_list.end(), hit_ptr->key()) != used_list.end()){
      continue; 
    }

    int seed_arm = hit_ptr->arm(); 
    double seed_shx = hit_ptr->x()/(hit_ptr->z()-_vertex);
    double seed_shy = hit_ptr->y()/(hit_ptr->z()-_vertex);
    double saved_seed_shx = seed_shx; 
    double saved_seed_shy = seed_shy; 
    double prev_seed_shx = -9999.0; 
    double prev_seed_shy = -9999.0; 
    double prev_shower_e = 0.0; 

    int num_iter = 0; 

    // Choose iterators based on seed_arm, avoid looping over hits 
    // from the unmatched arm
    TMpcExHitSet<OrderByEnergy>::const_iterator last_inner;
    if(seed_arm==0)
      last_inner = last_arm0;
    else
      last_inner = last_arm1;

    // Expansion Factor for second stage shower expansion 
    double ExpansionFactor = 1.0;
    bool Stage1Complete = false; 
    bool Stage2Complete = false; 

    // Iterate starting with the seed until stable
    do{

      double shower_shx = 0.0; 
      double shower_shy = 0.0; 
      double shower_Norm_X = 0.0; 
      double shower_Norm_Y = 0.0; 
      double shower_e = 0.0; 
      current_used_list.clear(); 

      // Choose iterators based on seed_arm, avoid looping over hits 
      // from the unmatched arm
      TMpcExHitSet<OrderByEnergy>::const_iterator itr_inner;
      if(seed_arm==0)
	itr_inner = itr_arm0; 
      else
	itr_inner = itr_arm1; 

      for(; itr_inner!=last_inner; ++itr_inner){

	TMpcExHit* hit_ptr2 = *itr_inner;

	// only good energy calibrated hits
	if(!hit_ptr2->isGoodCombinedHit()) {
	  used_list.push_back(hit_ptr2->key());
	  continue;
	}

	// Only use *active* strip layers (user defined)
	short layer = hit_ptr2->layer();
	if(fLayerValid[layer] == 0) { 	    
	  used_list.push_back(hit_ptr2->key());
	  continue; 
	} 

	// make sure this hit isn't already used
	if(std::find(used_list.begin(), used_list.end(), hit_ptr2->key()) != used_list.end()){
	  continue; 
	}

	double q_hit = hit_ptr2->combined(); 
	double hx = hit_ptr2->x()/(hit_ptr2->z()-_vertex);
	double hy = hit_ptr2->y()/(hit_ptr2->z()-_vertex);

	// Calculate distance in hough space
	float dist = sqrt( (seed_shx - hx)*(seed_shx - hx) + 
			   (seed_shy - hy)*(seed_shy - hy));  

	// Check if hit is in the cone
	if(dist<(ExpansionFactor*shower_radius[seed_arm][layer])){ 

	  double sigma = (hit_ptr2->minipad_x_width()/sqrt(12.0))/fabs(hit_ptr2->z()-_vertex); 
	  double weight_X = q_hit*(1.0/(sigma*sigma));
	  sigma = (hit_ptr2->minipad_y_width()/sqrt(12.0))/fabs(hit_ptr2->z()-_vertex); 
	  double weight_Y = q_hit*(1.0/(sigma*sigma));

	  shower_shx += hx*weight_X; 
	  shower_shy += hy*weight_Y;  
	  shower_Norm_X += weight_X; 
	  shower_Norm_Y += weight_Y; 

	  shower_e += q_hit; 

	  current_used_list.push_back(hit_ptr2->key());
	    
	}

      }

      // calculate new seed parameters
      prev_seed_shx = seed_shx; 
      prev_seed_shy = seed_shy; 

      // avoid divide by zero
      if(!Stage1Complete && current_used_list.size()>0){

	// Stage 1: determine the seed with a narrow shower 
	// radius until the seed stops changing

	seed_shx = shower_shx/shower_Norm_X; 
	seed_shy = shower_shy/shower_Norm_Y;
	
	if((fabs(seed_shx-prev_seed_shx)<=HOUGH_CONVERGE) || 
	   (fabs(seed_shy-prev_seed_shy)<=HOUGH_CONVERGE)) {
	  Stage1Complete = true; 
	  ExpansionFactor = EXPANSION_START; 
	  prev_shower_e = shower_e; 
	}

      }
      else{
	
	// Stage2 - don't update the seed but expand the
	// radius until the fractional energy change falls below 
	// the limit. 
	
	double frac_gain = (shower_e-prev_shower_e)/prev_shower_e; 

	if( (frac_gain<FRACTIONAL_ENERGY_LIMIT) || (ExpansionFactor>=EXP_FACTOR_LIMIT)){
	  Stage2Complete = true; 
	  // Update to the final seed
	  seed_shx = shower_shx/shower_Norm_X; 
	  seed_shy = shower_shy/shower_Norm_Y; 
	}
	else{
	  ExpansionFactor *= EXPANSION_START; 
	  prev_shower_e = shower_e; 
	}

      }

      num_iter++;

    }while( !Stage2Complete && 
	    (num_iter<MAX_ITERATIONS) && 
	    (current_used_list.size()>0));

    // Protection - sometimes you can get hits at two different edges of the 
    // shower radius that gives you a centroid that excludes both, and you end
    // with no hits assigned after iteration. 
    // Also eliminate single minipad hits (noise). 
    if( current_used_list.size()<=1 ) {
      // Add current seed hit to used list and continue
      used_list.push_back(hit_ptr->key());
      continue;
    }

    // Record used hits, add to clusters and showers
    TMpcExShower *shower_v1 = new TMpcExShower(seed_arm);
    if(!shower_v1){
      std::cout << PHWHERE << " Error allocating new TMpcExShower, exiting." << std::endl; 
      exit(-1); 
    }
    _shower_map->addShower(shower_v1); 

    shower_v1->set_arm(seed_arm); 
    shower_v1->set_hsx(seed_shx); 
    shower_v1->set_hsy(seed_shy); 
    shower_v1->set_seed_hsx(saved_seed_shx); 
    shower_v1->set_seed_hsy(saved_seed_shy); 
    shower_v1->set_merged(0); 
    shower_v1->set_delete_me(0); 
    shower_v1->set_n_iter(num_iter); 
    shower_v1->set_vertex(_vertex); 
    shower_v1->set_ExpansionFactor(ExpansionFactor); 

    for(unsigned int i=0; i<current_used_list.size(); i++){
      //Associate these hits to the shower      
      shower_v1->addHit(current_used_list[i]); 
      used_list.push_back(current_used_list[i]); 
    }

    // Bail out if all hits are used
    if(used_list.size()==_hit_map->size()) break; 

  }
    
  // Calculate the full shower properties 

  for(unsigned int shwrNum0=0; shwrNum0<_shower_map->size() ; shwrNum0++) {
    TMpcExShower *shower_v1 = _shower_map->getShower(shwrNum0); 
    CalculateShowerProperties(shower_v1);
    shower_v1->BuildContributorListMC(top_node); 
  }

  // Loop over showers to set the closestMPC flag 

  for(unsigned int shwrNum0=0; shwrNum0<_shower_map->size() ; shwrNum0++) {
    TMpcExShower *shower_v10 = _shower_map->getShower(shwrNum0); 

    if(shower_v10->get_ClosestMPCClusterIndex()<0) continue; // no associated cluster

    bool closest = true; 
    for(unsigned int shwrNum1=0; shwrNum1<_shower_map->size() ; shwrNum1++) {
      TMpcExShower *shower_v11 = _shower_map->getShower(shwrNum1);
      if(shwrNum0==shwrNum1) continue; // don't compare shower to itself. 
      // not the same cluster
      if(shower_v11->get_ClosestMPCClusterIndex()!=shower_v10->get_ClosestMPCClusterIndex()) continue;  
      if(shower_v11->get_ClosestMPCClusterDistance() < shower_v10->get_ClosestMPCClusterDistance()){
  	closest = false; 
  	break; 
      }
    }
    if(closest) shower_v10->set_ClosestMPCClusterClosestFlag(1); 

  }

  return True;
}

bool mMpcExShowerV2::FindShowersV2(PHCompositeNode* top_node){ 
  int box_2d_array[2][nbox][nbox];
  memset(box_2d_array,-1,2*nbox*nbox*sizeof(int));

  double hs_ex_size[2] = {ex_size/fabs(-ex_z-_vertex),ex_size/fabs(ex_z-_vertex)};
  double hs_box_size[2] = {box_size/fabs(-ex_z-_vertex),box_size/fabs(ex_z-_vertex)};

  unsigned int nhits = _hit_map->size();
  if(nhits==0) return true;
  
//  std::cout<<"box process"<<std::endl;
  for(unsigned int ihit=0;ihit<nhits;ihit++){ 
    TMpcExHit* hit = _hit_map->getHit(ihit);
    int arm = hit->arm();
     //init the union element
    ele_array[ihit].init(ihit);
    if(!hit->isGoodCombinedHit()) continue;
    if(DisableNorth && arm==1) continue;
    if(DisableSouth && arm==0) continue;

    double x = hit->x();
    double y = hit->y();
    double z = hit->z();
    double x_w = hit->minipad_x_width()/2.;
    double y_w = hit->minipad_y_width()/2.;
    double hsx = x/(z-_vertex);
    double hsy = y/(z-_vertex);
        
	
    //mark the aera of minipad to be true
    double hsx_w = x_w/fabs(z-_vertex);
    double hsy_w = y_w/fabs(z-_vertex);
    int lim_ix0 = (hsx-hsx_w+hs_ex_size[arm])/hs_box_size[arm]; 
    int lim_ix1 = (hsx+hsx_w+hs_ex_size[arm])/hs_box_size[arm];
    int lim_iy0 = (hsy-hsy_w+hs_ex_size[arm])/hs_box_size[arm]; 
    int lim_iy1 = (hsy+hsy_w+hs_ex_size[arm])/hs_box_size[arm];
        
	//check the surrondings
    for(int s=lim_ix0-1;s<=lim_ix1+1;s++){ 
      if(s<0 || s>nbox-1) continue;
      for(int t=lim_iy0-1;t<=lim_iy1+1;t++){ 
	if(t<0 || t>nbox-1) continue;
	//box is occupied
	if(box_2d_array[arm][s][t]>=0){
	  int root1 = get_root(box_2d_array[arm][s][t]);
	  int root2 = get_root(ihit);
	  //if the root are the same, already merged
	  //only consider the root are different
	  if(root1 != root2){ 
            merge_UnionElement(root1,root2); 
	  }
	}
	else{
          //not occupied
	  //lim_ix0-1 or lim_ix1+1, is neighbor box
	  //not belonging to this pad
	  if(s>=lim_ix0 && s<=lim_ix1 && t>=lim_iy0 && t<=lim_iy1){
            box_2d_array[arm][s][t] = ihit;
	  }
	}
      }//t
    }//s
  }//ihit


  //step two merge the box
  //will use dfs search
  //index is a map from ihit to shower
  TMpcExShower* shower_array[nhits];
  memset(shower_array,0,nhits*sizeof(TMpcExShower*));

  double shower_hsx[nhits];
  memset(shower_hsx,0,nhits*sizeof(double));
  double shower_hsy[nhits];
  memset(shower_hsy,0,nhits*sizeof(double));
  double shower_norm_x[nhits];
  memset(shower_norm_x,0,nhits*sizeof(double));
  double shower_norm_y[nhits];
  memset(shower_norm_y,0,nhits*sizeof(double));
  vector<int> root_list;
  
//  std::cout<<"shower process"<<endl;
  for(unsigned int ihit=0;ihit<nhits;ihit++){
    int root = get_root(ihit);
    //ignore only one hit shower
    if(ele_array[root].num<=1) continue;
    
    TMpcExHit* hit = _hit_map->getHit(ihit);
    int arm = hit->arm();
    
    //get shower hsx and hsy
    double q_hit = hit->combined(); 
    double hx = hit->x()/(hit->z()-_vertex);
    double hy = hit->y()/(hit->z()-_vertex);

    double sigma = (hit->minipad_x_width()/sqrt(12.0))/fabs(hit->z()-_vertex); 
    double weight_X = q_hit*(1.0/(sigma*sigma));
    sigma = (hit->minipad_y_width()/sqrt(12.0))/fabs(hit->z()-_vertex); 
    double weight_Y = q_hit*(1.0/(sigma*sigma));


    //the root can be larger than ihit
    if(!shower_array[root]){ 
      shower_array[root] = new TMpcExShower(arm);
      _shower_map->addShower(shower_array[root]);
      root_list.push_back(root);
    }
    shower_array[root]->addHit(hit->key());
    shower_hsx[root] += hx*weight_X; 
    shower_hsy[root] += hy*weight_Y;  
    shower_norm_x[root] += weight_X; 
    shower_norm_y[root] += weight_Y; 
  }
  
  for(unsigned int irt=0;irt<root_list.size();irt++){ 
    int ishower = root_list[irt];
    shower_hsx[ishower] = shower_hsx[ishower]/shower_norm_x[ishower];
    shower_hsy[ishower] = shower_hsy[ishower]/shower_norm_y[ishower];
    shower_array[ishower]->set_hsx(shower_hsx[ishower]);
    shower_array[ishower]->set_hsy(shower_hsy[ishower]);
  }


  // Calculate the full shower properties 
  
//  std::cout<<"property process"<<endl;
  for(unsigned int shwrNum0=0; shwrNum0<_shower_map->size() ; shwrNum0++) {
    TMpcExShower *shower_v1 = _shower_map->getShower(shwrNum0); 
    CalculateShowerProperties(shower_v1);
    shower_v1->BuildContributorListMC(top_node); 
  }

  // Loop over showers to set the closestMPC flag 

  for(unsigned int shwrNum0=0; shwrNum0<_shower_map->size() ; shwrNum0++) {
    TMpcExShower *shower_v10 = _shower_map->getShower(shwrNum0); 

    if(shower_v10->get_ClosestMPCClusterIndex()<0) continue; // no associated cluster

    bool closest = true; 
    for(unsigned int shwrNum1=0; shwrNum1<_shower_map->size() ; shwrNum1++) {
      TMpcExShower *shower_v11 = _shower_map->getShower(shwrNum1);
      if(shwrNum0==shwrNum1) continue; // don't compare shower to itself. 
      // not the same cluster
      if(shower_v11->get_ClosestMPCClusterIndex()!=shower_v10->get_ClosestMPCClusterIndex()) continue;  
      if(shower_v11->get_ClosestMPCClusterDistance() < shower_v10->get_ClosestMPCClusterDistance()){
  	closest = false; 
  	break; 
      }
    }
    if(closest) shower_v10->set_ClosestMPCClusterClosestFlag(1); 

  }

  return true;
}


void  mMpcExShowerV2::MergeShowers(TMpcExShower *main, TMpcExShower *splinter)
{
  // Add the splinter hits to the main shower

  unsigned int nhits = splinter->sizeHits(); 
  for(unsigned int hitNum=0; hitNum<nhits ; hitNum++) main->addHit(splinter->getHit(hitNum));
  main->set_merged(1); 
  splinter->set_delete_me(1); 
  
}

void  mMpcExShowerV2::CalculateShowerProperties(TMpcExShower *shower_v1){

      double e_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      int    n_layer[MpcExConstants::NLAYERS] = {0, 0, 0, 0, 0, 0, 0, 0}; 
      double rms_hsx_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double rms_hsy_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double sum_weight_X_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double sum_weight_Y_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double disp_hsx_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double disp_hsy_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double sum_weight_X_D_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double sum_weight_Y_D_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 

      double x_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double y_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double z_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 

      double x_layer_err[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double y_layer_err[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 

      double hsx_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      double hsy_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


      float x_shower = 0.0; 
      float y_shower = 0.0; 
      float z_shower = 0.0; 

      // Calculate the RMS and DISPERSION of the shower
      // This also calculates the positions arrays 

      // First loop for the RMS
      unsigned int n_sat_minipads = 0; 
      unsigned int nhits = shower_v1->sizeHits(); 
      for(unsigned int i=0; i<nhits ; i++){

//	  TMpcExHit *hit_ptr3 = _hit_map->get_hit_by_key( shower_v1->getHit(i) );
          
	  TMpcExHit *hit_ptr3 = _hit_map->getUncalHit( shower_v1->getHit(i) );
//	  if(hit_ptr3 != hit_ptr3_test){ 
//	    std::cout<<"  key different "<<hit_ptr3->key()<<"  "<<hit_ptr3_test->key()<<std::endl;
//	  }


	  double q_hit = hit_ptr3->combined(); 
	  short layer = hit_ptr3->layer();
	  
	  // set for simulation only for now
	  if( q_hit >= ((255-20.0)*(147.6/19.0)*4.5) ) n_sat_minipads++; 

	  e_layer[layer] +=  q_hit;
	  n_layer[layer]++; 

	  x_shower += hit_ptr3->x()*q_hit; 
	  y_shower += hit_ptr3->y()*q_hit; 
	  z_shower += hit_ptr3->z()*q_hit; 

	  // RMS

	  double sigma = (hit_ptr3->minipad_x_width()/sqrt(12.0))/fabs(hit_ptr3->z()-_vertex); 
	  double weight_X = q_hit*(1.0/(sigma*sigma));
	  sigma = (hit_ptr3->minipad_y_width()/sqrt(12.0))/fabs(hit_ptr3->z()-_vertex); 
	  double weight_Y = q_hit*(1.0/(sigma*sigma));
	  
	  rms_hsx_layer[layer] +=  pow((hit_ptr3->x()/(hit_ptr3->z() - _vertex) - 
			   shower_v1->get_hsx()),2)*weight_X; 

	  rms_hsy_layer[layer]  += pow((hit_ptr3->y()/(hit_ptr3->z() - _vertex) - 
			   shower_v1->get_hsy()),2)*weight_Y; 

	  double hx = hit_ptr3->x()/(hit_ptr3->z()-_vertex);
	  double hy = hit_ptr3->y()/(hit_ptr3->z()-_vertex);
	  hsx_layer[layer] += hx*weight_X;
	  hsy_layer[layer] += hy*weight_Y;

	  sum_weight_X_layer[layer] += weight_X; 
	  sum_weight_Y_layer[layer] += weight_Y; 

	  x_layer[layer] += hit_ptr3->x()*weight_X;
	  y_layer[layer] += hit_ptr3->y()*weight_Y;
	  z_layer[layer] += hit_ptr3->z()*q_hit;

	  x_layer_err[layer] += pow(weight_X*(hit_ptr3->minipad_x_width()/sqrt(12.0)),2); 
	  y_layer_err[layer] += pow(weight_Y*(hit_ptr3->minipad_y_width()/sqrt(12.0)),2); 
  
      }

      shower_v1->set_n_sat_minipads(n_sat_minipads); 

      double eweight = 0.0; 
      for(int i=0; i<MpcExConstants::NLAYERS; i++) eweight += e_layer[i]; 
      
      x_shower /= eweight; 
      y_shower /= eweight; 
      z_shower /= eweight; 

      shower_v1->set_ew_x(x_shower); 
      shower_v1->set_ew_y(y_shower); 
      shower_v1->set_ew_z(z_shower); 

      // SECOND loop for the DISPERSION

      for(unsigned int i=0; i<nhits ; i++){

//	  TMpcExHit *hit_ptr3 = _hit_map->get_hit_by_key( shower_v1->getHit(i) );

	  TMpcExHit *hit_ptr3 = _hit_map->getUncalHit( shower_v1->getHit(i) );
	//  if(hit_ptr3->key() != hit_ptr3_test->key()){ 
	//    std::cout<<"  key different "<<hit_ptr3->key()<<"  "<<hit_ptr3_test->key()<<std::endl;
	//  }


	  double q_hit = hit_ptr3->combined(); 
	  short layer = hit_ptr3->layer();

	  // Dispersion

	  double sigma = (hit_ptr3->minipad_x_width()/sqrt(12.0))/fabs(hit_ptr3->z()-_vertex); 
	  double weight_X = TMath::Max(0.0,(4.5+log(q_hit/eweight))*(1.0/(sigma*sigma)));
	  sigma = (hit_ptr3->minipad_y_width()/sqrt(12.0))/fabs(hit_ptr3->z()-_vertex); 
	  double weight_Y = TMath::Max(0.0,(4.5+log(q_hit/eweight))*(1.0/(sigma*sigma)));

	  disp_hsx_layer[layer] +=  pow((hit_ptr3->x()/(hit_ptr3->z() - _vertex) - 
			    shower_v1->get_hsx()),2)*weight_X; 

	  disp_hsy_layer[layer] +=  pow((hit_ptr3->y()/(hit_ptr3->z() - _vertex) - 
			    shower_v1->get_hsy()),2)*weight_Y; 

	  sum_weight_X_D_layer[layer] += weight_X; 
	  sum_weight_Y_D_layer[layer] += weight_Y; 

      }

      double rms_hsx = 0.0; 
      double rms_hsy = 0.0; 
      double disp_hsx = 0.0; 
      double disp_hsy = 0.0; 
      double sum_weight_X = 0.0; 
      double sum_weight_Y = 0.0; 
      double sum_weight_X_D = 0.0; 
      double sum_weight_Y_D = 0.0;  
      double r_layer[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
      double r_layer_err[MpcExConstants::NLAYERS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
     
      if(shower_v1->sizeHits()>1){

	for(int i=0; i<MpcExConstants::NLAYERS; i++){
	  
	  rms_hsx += rms_hsx_layer[i]; 
	  rms_hsy += rms_hsy_layer[i]; 

	  sum_weight_X += sum_weight_X_layer[i]; 
	  sum_weight_Y += sum_weight_Y_layer[i]; 

	  disp_hsx += disp_hsx_layer[i]; 
	  disp_hsy += disp_hsy_layer[i]; 

	  sum_weight_X_D += sum_weight_X_D_layer[i]; 
	  sum_weight_Y_D += sum_weight_Y_D_layer[i]; 

	  if(e_layer[i]>0){

	    rms_hsx_layer[i] = sqrt(rms_hsx_layer[i]/sum_weight_X_layer[i]); 
	    rms_hsy_layer[i] = sqrt(rms_hsy_layer[i]/sum_weight_Y_layer[i]); 
	    disp_hsx_layer[i] = sqrt(disp_hsx_layer[i]/sum_weight_X_D_layer[i]); 
	    disp_hsy_layer[i] = sqrt(disp_hsy_layer[i]/sum_weight_Y_D_layer[i]); 
	    
	    x_layer[i] = x_layer[i]/sum_weight_X_layer[i]; 
	    y_layer[i] = y_layer[i]/sum_weight_Y_layer[i]; 
	    z_layer[i] = z_layer[i]/e_layer[i]; 

	    x_layer_err[i] = sqrt(x_layer_err[i])/sum_weight_X_layer[i]; 
	    y_layer_err[i] = sqrt(y_layer_err[i])/sum_weight_Y_layer[i]; 

	    r_layer[i] = sqrt(pow(x_layer[i],2) + pow(y_layer[i],2)); 
	    r_layer_err[i] = sqrt(pow(x_layer[i]*x_layer_err[i],2) + pow(y_layer[i]*y_layer_err[i],2))/r_layer[i]; 

	    hsx_layer[i] = hsx_layer[i]/sum_weight_X_layer[i]; 
	    hsy_layer[i] = hsy_layer[i]/sum_weight_Y_layer[i]; 	    

	  }
	  else{
	    
	    rms_hsx_layer[i] = 0.0; 
	    rms_hsy_layer[i] = 0.0; 
	    disp_hsx_layer[i] = 0.0; 
	    disp_hsy_layer[i] = 0.0; 

	    hsx_layer[i] = 0.0; 
	    hsy_layer[i] = 0.0; 	    

	  }

	}

	rms_hsx = sqrt(rms_hsx/sum_weight_X);
	rms_hsy = sqrt(rms_hsy/sum_weight_Y);

	disp_hsx = sqrt(disp_hsx/sum_weight_X_D);
	disp_hsy = sqrt(disp_hsy/sum_weight_Y_D);

      }
      else{

	rms_hsx = 0.0;
	rms_hsy = 0.0;

	disp_hsx = 0.0;
	disp_hsy = 0.0;

	for(int i=0; i<MpcExConstants::NLAYERS; i++){
	  rms_hsx_layer[i] = 0.0; 
	  rms_hsy_layer[i] = 0.0; 
	  disp_hsx_layer[i] = 0.0; 
	  disp_hsy_layer[i] = 0.0; 
	  hsx_layer[i] = 0.0; 
	  hsy_layer[i] = 0.0; 	    	  
	}

      }
      
      // Calculate number of layers, first layer
      int nlayers = 0;
      int first_layer = MpcExConstants::NLAYERS;
      for(int layer=0; layer<MpcExConstants::NLAYERS; layer++){
	if(e_layer[layer]>0.0) {
	  if(first_layer == MpcExConstants::NLAYERS) first_layer = layer; 
	  nlayers++;
	}
      }

      shower_v1->set_nlayers(nlayers);
      shower_v1->set_first_layer(first_layer); 

      // Fill in the shower information
      shower_v1->set_rms_hsx(rms_hsx); 
      shower_v1->set_rms_hsy(rms_hsy); 
      shower_v1->set_disp_hsx(disp_hsx); 
      shower_v1->set_disp_hsy(disp_hsy); 
      shower_v1->set_raw_esum(eweight/1.0e6);    // 1.0e6 convert keV->GeV
                 
      for(int layer=0; layer< MpcExConstants::NLAYERS; layer++){
	shower_v1->set_e_layer(layer,e_layer[layer]/1.0e6); 
	shower_v1->set_n_layer(layer,n_layer[layer]); 
	shower_v1->set_rms_hsx_layer(layer,rms_hsx_layer[layer]); 
	shower_v1->set_rms_hsy_layer(layer,rms_hsy_layer[layer]); 
	shower_v1->set_disp_hsx_layer(layer,disp_hsx_layer[layer]); 
	shower_v1->set_disp_hsy_layer(layer,disp_hsy_layer[layer]);
	shower_v1->set_hsx_layer(layer,hsx_layer[layer]); 
	shower_v1->set_hsy_layer(layer,hsy_layer[layer]); 
      }
      
      // Collect the r,z layer information 
      // Need to fill hit arrays as the shower information might not be contiguous
      
      float r_fit[MpcExConstants::NLAYERS]; 
      float r_fit_err[MpcExConstants::NLAYERS]; 
      float z_fit[MpcExConstants::NLAYERS]; 
      float z_fit_err[MpcExConstants::NLAYERS];
      int n_fit=0;
      for(int layer=first_layer; layer< MpcExConstants::NLAYERS; layer++){
	if(e_layer[layer]>0.0){
	  r_fit[n_fit] = r_layer[layer];
	  r_fit_err[n_fit] = r_layer_err[layer];
	  z_fit[n_fit] = z_layer[layer];
	  z_fit_err[n_fit] = 0.1; // 1.0 mm assumed z error
	  n_fit++; 
	}
      }
      shower_v1->set_fit_data(n_fit,r_fit,z_fit,r_fit_err,z_fit_err); 
      
      // MPC information 
            
      int iMPCTwr_peak = -1; 
      double MPCE[5][5]; 
      double MPCTOF[5][5]; 
      double MPCHS[5][5][2]; 
      int N_3x3, N_5x5;
      int fiducial; 
      int pkix, pkiy; 
      double MPCQUAL[5][5]; 
      getMPC_TwrSum(shower_v1->get_arm(), shower_v1->get_hsx(), shower_v1->get_hsy(), iMPCTwr_peak, MPCE, MPCTOF, MPCHS, MPCQUAL,   
		    N_3x3, N_5x5, fiducial, pkix, pkiy); 

      shower_v1->set_mpcN3x3(N_3x3); 
      shower_v1->set_mpcN5x5(N_5x5); 
      shower_v1->set_mpcCentTwr(iMPCTwr_peak); 
      shower_v1->set_fiducial(fiducial); 
      shower_v1->set_mpcPeakix(pkix); 
      shower_v1->set_mpcPeakiy(pkiy); 

      for(int i=0; i<5; i++){
	for(int j=0; j<5; j++){
	  shower_v1->set_mpcTwrE(i,j,MPCE[i][j]);
	  shower_v1->set_mpcTwrTOF(i,j,MPCTOF[i][j]);
	  shower_v1->set_mpcTwrHS(i,j,MPCHS[i][j][0],MPCHS[i][j][1]);
	  shower_v1->set_mpc_quality(i,j,MPCQUAL[i][j]);
	} 
      }

      // Use the energy calibration object

      _CalEnergy->CalibrateEnergy(shower_v1); 
          
      // Assign the closest MPC cluster to each shower and mark the shower closest to the cluster. 
      float dist, distx, disty, clustE; 
      int clustNum; 
      MatchMPC(shower_v1->get_arm(), shower_v1->get_hsx(), shower_v1->get_hsy(), dist, distx, disty, clustE, clustNum); 

      shower_v1->set_ClosestMPCClusterDistance(dist); 
      shower_v1->set_ClosestMPCClusterDistanceX(distx); 
      shower_v1->set_ClosestMPCClusterDistanceY(disty); 
      shower_v1->set_ClosestMPCClusterEnergy(clustE); 
      shower_v1->set_ClosestMPCClusterIndex(clustNum);

      // DO NOT set ClosestMPCClusterClosestFlag here
}

void  mMpcExShowerV2::SingleTrackReco()
{

  // CURRENTLY DEPRECATED AND UNUSED
  // JGL 07/23/2016

}

void mMpcExShowerV2::getMPC_TwrSum(int arm, double shx, double shy, int &iMPCTwr_peak, double MPCE[5][5], 
				 double MPCTOF[5][5], double MPCHS[5][5][2], double MPCQUAL[5][5],
				 int &N_3x3, int &N_5x5, int &fiducial, int &pkix, int &pkiy ){

  // Initialize returned shower properties

  pkix = -1; 
  pkiy = -1;
  N_3x3 = 0; 
  N_5x5 = 0; 

  for(int i=0; i<5; i++){
    for(int j=0; j<5; j++){
      MPCE[i][j] = 0.0; 
      MPCTOF[i][j] = -9999.0; 
      MPCHS[i][j][0] = -9999.0; 
      MPCHS[i][j][1] = -9999.0;
      MPCQUAL[i][j] = -9999.0; 
    } 
  }

  fiducial = 0;

  // Sum the MPC calibrated tower energy in a 3x3 and 5x5 around the 
  // MPC-EX track direction.
  
  double peak_dist = 9999.0;
  iMPCTwr_peak = -1; 
  int gridx_peak = -1;
  int gridy_peak = -1; 

  // Loop over all valid towers and look for the closest match to 
  // the MPC-EX projection 

  for(int ix=0; ix<18; ix++){
    for(int iy=0; iy<18; iy++){

      int tow_ch = _mpc_map->getFeeCh(ix,iy,arm);
      if(tow_ch>=0){

	float xloc = _mpc_map->getX(tow_ch); 
	float yloc = _mpc_map->getY(tow_ch); 
	float zloc = 0.0; 

	if(_mpc_map->getArm(tow_ch)==0)
	  zloc = -MpcExConstants::MPC_REFERENCE_Z; 
	else
	  zloc = MpcExConstants::MPC_REFERENCE_Z; 

	float mpc_hx = xloc/(zloc-_vertex); 
	float mpc_hy = yloc/(zloc-_vertex); 

	float dist = sqrt( (shx-mpc_hx)*(shx-mpc_hx) + (shy-mpc_hy)*(shy-mpc_hy) );  

	// Check if the tower is closest:

	if(dist<peak_dist) {
	  peak_dist = dist; 
	  gridx_peak = ix; 
	  gridy_peak = iy;  
	}

      }
    }
  }

  // Now check the region around the projected tower for the 
  // highest energy tower. Because showers enter the MPC at an angle, it
  // is possible that if you project to an edge of one tower, the peak energy could
  // be one tower over (based on angle of entry).  The check is a little different for
  // each quadrant. 

  if((gridx_peak>=0) && (gridy_peak>=0)) {

    int new_gridx_peak = gridx_peak; 
    int new_gridy_peak = gridy_peak; 
    float big_E = 0.0; 
    for(int ix=-1; ix<2; ix++){
      for(int iy=-1; iy<2; iy++){

	// only towers that exist
	int tow_ch = _mpc_map->getFeeCh(gridx_peak + ix,gridy_peak + iy,arm);
	if(tow_ch<0) continue; 

	// Match this tow_ch with the tower container to get the energy

	float this_E = 0.0; 
	int fNMpcTowers = _mpc_tower_container->size();
	for(int iMPCTwr=0 ; iMPCTwr<fNMpcTowers ; iMPCTwr++) {
	  mpcTowerContent *twr = _mpc_tower_container->getTower(iMPCTwr);
	  if(tow_ch!=twr->get_ch()) continue; 
	  this_E = twr->get_energy();
	  break; 
	}
 
	// searches by quadrant

	if((gridx_peak<9)&&(gridy_peak<9)){
	  if( (ix>0)||(iy>0) ) continue; 
	}
	else if((gridx_peak>=9)&&(gridy_peak<9)){
	  if( (ix<0)||(iy>0) ) continue; 
	}
	else if((gridx_peak<9)&&(gridy_peak>=9)){
	  if( (ix>0)||(iy<0) ) continue; 
	}
	else if((gridx_peak>=9)&&(gridy_peak>=9)){
	  if( (ix<0)||(iy<0) ) continue; 
	}

	if( (this_E>0.0) && (this_E>big_E) ){
	  new_gridx_peak = gridx_peak + ix; 
	  new_gridy_peak = gridy_peak + iy; 
	  big_E = this_E; 
	}
      }
    }

    pkix = new_gridx_peak; 
    pkiy = new_gridy_peak; 
    gridx_peak = new_gridx_peak; 
    gridy_peak = new_gridy_peak; 
	
    // Now loop over the towers, and calculate the energy sums

    int fNMpcTowers = _mpc_tower_container->size();

    for(int iMPCTwr=0 ; iMPCTwr<fNMpcTowers ; iMPCTwr++) {

      mpcTowerContent *twr = _mpc_tower_container->getTower(iMPCTwr);
  
      double mpc_energy = twr->get_energy();

      int tow_ch = twr->get_ch(); 

      // don't match if not the same arm!!
      if(_mpc_map->getArm(tow_ch)!= arm) continue; 

      int gridx = _mpc_map->getGridX(tow_ch); 
      int gridy = _mpc_map->getGridY(tow_ch); 

      if((gridx==gridx_peak) && (gridy==gridy_peak)) 
	iMPCTwr_peak = iMPCTwr; 

      if( (abs(gridx-gridx_peak)<=2) && (abs(gridy-gridy_peak)<=2) ) {

	// if(pFlag){
	//  cout << gridx-gridx_peak+2 << " " << gridy-gridy_peak+2 << " " << mpc_energy << endl; 
	// }

        if( (std::isnan(mpc_energy)==0) && (mpc_energy>0.0) ){
	  MPCE[gridx-gridx_peak+2][gridy-gridy_peak+2] = mpc_energy;
	  MPCTOF[gridx-gridx_peak+2][gridy-gridy_peak+2] = twr->get_tof() - _t0;

	  // Match this up with the raw tower and pull out the quality flag 

	  float fqual = -9999.0; 
	  if(_mpcraw2_container){
	    for (unsigned int iraw=0; iraw<_mpcraw2_container->size(); iraw++){
	      mpcRawContent *raw = (mpcRawContent *)_mpcraw2_container->getTower(iraw);
	      if(raw->get_ch()==twr->get_ch()){
		fqual = raw->get_fquality();
		break; 
	      }
	    }
	  }
	  MPCQUAL[gridx-gridx_peak+2][gridy-gridy_peak+2] = fqual;

	} 
	else{
	  MPCE[gridx-gridx_peak+2][gridy-gridy_peak+2] = 0.0;
	  MPCTOF[gridx-gridx_peak+2][gridy-gridy_peak+2] = 0.0;
	  MPCQUAL[gridx-gridx_peak+2][gridy-gridy_peak+2] = -9999.0;
	}
	
      }
      
    }

    // Need to loop differently to get the number of valid crystals 
    // as on the DST the tower content only contains towers with nonzero energy

    for(int ix=-2; ix<=2; ix++){
      for(int iy=-2; iy<=2; iy++){
	int tow_ch = _mpc_map->getFeeCh(gridx_peak+ix,gridy_peak+iy,arm);
	if(tow_ch>=0){
	  N_5x5++;
	  if((ix>=-1) && (ix<=1) && (iy>=-1) && (iy<=1)) N_3x3++;

	  float xloc = _mpc_map->getX(tow_ch); 
	  float yloc = _mpc_map->getY(tow_ch); 
	  float zloc = 0.0; 

	  if(_mpc_map->getArm(tow_ch)==0)
	    zloc = -MpcExConstants::MPC_REFERENCE_Z; 
	  else
	    zloc = MpcExConstants::MPC_REFERENCE_Z; 

	  MPCHS[ix+2][iy+2][0] = xloc/(zloc-_vertex);  
	  MPCHS[ix+2][iy+2][1] = yloc/(zloc-_vertex);  

	}
      }
    }

    if(N_3x3==9) fiducial = 1; 

  }

}

void mMpcExShowerV2::MatchMPC(int arm, double fMPCEXHoughX, double fMPCEXHoughY, float &fMPC_dhough, 
			     float &distx, float &disty, float &clustE, int &clustNum)
{

  fMPC_dhough=1.0;     // only match if within a reasonable range
  distx = 1.0; 
  disty = 1.0; 
  clustE = 0.0; 
  clustNum = -1; 

  if(!_mpc_cluster_container) return; 

  int fNMpcClusters = _mpc_cluster_container->size();

  for(int iMPCClus=0 ; iMPCClus<fNMpcClusters ; iMPCClus++) 
    {
      mpcClusterContent *clus =  _mpc_cluster_container->getCluster(iMPCClus);
  
      // don't match if not the same arm!!
      if(clus->arm()!=arm) continue; 

      double fThisDiff = pow(fMPCEXHoughX-(clus->x()/(clus->z()-_vertex)),2);
      fThisDiff       += pow(fMPCEXHoughY-(clus->y()/(clus->z()-_vertex)),2);
      fThisDiff = sqrt(fThisDiff);
      if( fThisDiff<fMPC_dhough ){
	  fMPC_dhough         = fThisDiff;
	  distx = fMPCEXHoughX-(clus->x()/(clus->z()-_vertex)); 
	  disty = fMPCEXHoughY-(clus->y()/(clus->z()-_vertex));
	  clustE = clus->e(); 
	  clustNum = iMPCClus; 
      }

    }

}

