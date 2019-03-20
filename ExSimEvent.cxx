#include "ExSimEvent.h"
#include <primaryWrapper.h>
#include <PHCompositeNode.h>
#include <getClass.h>
#include <PHGlobal.h>
#include <BbcOut.h>
#include <iostream>
#include <set>
#include <fkinWrapper.h>
using namespace std;

TrueParticle::TrueParticle(){
  _id = -9999;
  _parent_id = -9999;
  _vertex = -9999;
  _px = -9999;
  _py = -9999;
  _pz = -9999;
}

TrueParticle::TrueParticle(int id,int parent_id,float vertex,float px,float py,float pz):
_id(id),_parent_id(parent_id),_vertex(vertex),_px(px),_py(py),_pz(pz)
{
}

TruePi0::TruePi0(){
  TrueParticle();
  for(unsigned int i=0;i<2;i++){
    _gamma_vertex[i] = -9999;
    _gamma_px[i] = -9999;
    _gamma_py[i] = -9999;
    _gamma_pz[i] = -9999;
  }
}

TruePi0::TruePi0(int id,int parent_id,float vertex,float px,float py,float pz):
 TrueParticle(id,parent_id,vertex,px,py,pz)
{
  for(unsigned int i=0;i<2;i++){
    _gamma_vertex[i] = -9999;
    _gamma_px[i] = -9999;
    _gamma_py[i] = -9999;
    _gamma_pz[i] = -9999;
  }

}


ExSimEvent::ExSimEvent(){
  ExEvent();
  _true_particles.clear();
}

ExSimEvent::~ExSimEvent(){
  Reset();
}

void ExSimEvent::Clear(Option_t* option){
  ExEvent::Clear(option);
  Reset();
}

void ExSimEvent::Reset(){
  ExEvent::Reset();
  for(unsigned int i=0;i<_true_particles.size();i++){
    if(_true_particles[i]) delete _true_particles[i];
  }
  _true_particles.clear();
}

void ExSimEvent::AddFromPrimary(PHCompositeNode* simNode){
  if(!simNode){
    std::cout<<"Null Node !!!"<<std::endl;
    return;
  }

  PHGlobal* phglobal = findNode::getClass<PHGlobal>(simNode,"PHGlobal");
  BbcOut* bbcout = findNode::getClass<BbcOut>(simNode,"BbcOut");
  if(!bbcout && !phglobal){
    std::cout <<"No BbcOut or PHGlobal !!!"<<std::endl;
    return;
  }

  double vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();

  primaryWrapper* primary = findNode::getClass<primaryWrapper>(simNode, "primary");
  if(!primary){
    std::cout<<"No primaryWrapper !"<<std::endl;
    return;
  }
  size_t nprim = primary->RowCount();

  for(size_t iprim=0;iprim < nprim;iprim++){
    int idparticle = primary->get_idpart(iprim);
    double px = (-1)*primary->get_px_momentum(iprim);
    double py = primary->get_py_momentum(iprim);
    double pz = primary->get_pz_momentum(iprim);
       //no parent id, set to -1
    TrueParticle* particle = new TrueParticle(idparticle,-1,vertex,px,py,pz);
    _true_particles.push_back(particle);
  }
}

void ExSimEvent::AddPi0FromFkin(PHCompositeNode* simNode){
  fkinWrapper* fkinNode = findNode::getClass<fkinWrapper>(simNode,"fkin");
  TABLE_HEAD_ST fkin_h = fkinNode->TableHeader();
  FKIN_ST *fkin = fkinNode->TableData();
  double DEG_TO_RAD = 3.141593/180;

  typedef pair<int,int> PhotonPair;
  typedef pair<int,float> PhotonVertex;
  vector<PhotonPair> photon_pair_list;
  vector<PhotonVertex> photon_vertex_list;
  vector<int> primary_pi0_list;
  set<int> matched_pi0_paris;

  //record primary pi0s and pi0 decayed photons
  for(int fkin_index = 0; fkin_index < fkin_h.nok; fkin_index++){
    FKIN_ST& kin_hit(fkin[fkin_index]);
    int idparent = kin_hit.idparent;
    int id = kin_hit.idpart;
    
    //also remove large vertex 
    if(fabs(kin_hit.z_vertex)>100) continue;
     //record pi0 decayed photons
    if(idparent==7 && id==1){
      photon_vertex_list.push_back(PhotonVertex(fkin_index,kin_hit.z_vertex));
    }
    //record primary pi0
    if(idparent==0 && id==7){
      primary_pi0_list.push_back(fkin_index);
    }
  }
  
  //use the vertex to match photons
  set<int> matched_photon_set;
  for(unsigned int i0 = 0;i0<photon_vertex_list.size();i0++){
    for(unsigned int i1 = i0+1;i1<photon_vertex_list.size();i1++){
      int index0 = photon_vertex_list[i0].first;
      int index1 = photon_vertex_list[i1].first;

      if(matched_photon_set.find(index0) != matched_photon_set.end()) continue;
      if(matched_photon_set.find(index1) != matched_photon_set.end()) continue;


      float vtex0 = photon_vertex_list[i0].second;
      float vtex1 = photon_vertex_list[i1].second;
      //match photons pairs
      if(fabs(vtex0-vtex1)<1e-6){
        photon_pair_list.push_back(PhotonPair(index0,index1));
        matched_photon_set.insert(index0);
        matched_photon_set.insert(index1);
      }
    }
  }
  
  //match to primary pi0s using vertex and px, py and pz
  //and add to the true particle list
  for(unsigned int i0=0;i0<primary_pi0_list.size();i0++){
    for(unsigned int i1=0;i1<photon_pair_list.size();i1++){
      if(matched_pi0_paris.find(i1) != matched_pi0_paris.end()) continue;
      int pi0_index = primary_pi0_list[i0];
      int photon_index0 = photon_pair_list[i1].first;
      int photon_index1 = photon_pair_list[i1].second;

      FKIN_ST& kin_hit0(fkin[pi0_index]);
      FKIN_ST& kin_hit1(fkin[photon_index0]);
      FKIN_ST& kin_hit2(fkin[photon_index1]);

      double px0 = -kin_hit0.ptot*std::sin( DEG_TO_RAD*kin_hit0.pthet )*cos( DEG_TO_RAD*kin_hit0.pphi );
      double py0 = kin_hit0.ptot*std::sin( DEG_TO_RAD*kin_hit0.pthet )*sin( DEG_TO_RAD*kin_hit0.pphi );
      double pz0 = kin_hit0.ptot*std::cos( DEG_TO_RAD*kin_hit0.pthet );

      double px1 = -kin_hit1.ptot*std::sin( DEG_TO_RAD*kin_hit1.pthet )*cos( DEG_TO_RAD*kin_hit1.pphi );
      double py1 = kin_hit1.ptot*std::sin( DEG_TO_RAD*kin_hit1.pthet )*sin( DEG_TO_RAD*kin_hit1.pphi );
      double pz1 = kin_hit1.ptot*std::cos( DEG_TO_RAD*kin_hit1.pthet );

      double px2 = -kin_hit2.ptot*std::sin( DEG_TO_RAD*kin_hit2.pthet )*cos( DEG_TO_RAD*kin_hit2.pphi );
      double py2 = kin_hit2.ptot*std::sin( DEG_TO_RAD*kin_hit2.pthet )*sin( DEG_TO_RAD*kin_hit2.pphi );
      double pz2 = kin_hit2.ptot*std::cos( DEG_TO_RAD*kin_hit2.pthet );


      float pi0_vtex = fkin[pi0_index].z_vertex;

      float photon_vtex = fkin[photon_index0].z_vertex;

      if(fabs(pi0_vtex-photon_vtex)<0.01&&fabs(px0-px1-px2)<0.01&&fabs(py0-py1-py2)<0.01&&fabs(pz0-pz1-pz2)<0.01){
        TruePi0* pi0 = new TruePi0(7,0,pi0_vtex,px0,py0,pz0);
	pi0->SetGammaPx(px1,0);
	pi0->SetGammaPx(px2,1);
	pi0->SetGammaPy(py1,0);
	pi0->SetGammaPy(py2,1);
	pi0->SetGammaPz(pz1,0);
	pi0->SetGammaPz(pz2,1);
        pi0->SetGammaVertex(photon_vtex,0);
	pi0->SetGammaVertex(photon_vtex,1);
        _true_particles.push_back(pi0);
      }
    }
  }
}

