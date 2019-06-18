#include "ExPrimParticle.h"
#include <PHCompositeNode.h>
#include <getClass.h>
#include <primaryWrapper.h>
#include <BbcOut.h>
#include <PHGlobal.h>
#include <Fun4AllServer.h>
#include <fkinWrapper.h>
#include <map>
#include <iostream>

DecayParticle::DecayParticle(){
  _track_id=-9999;
  _parent_track_id = -9999;
  _id = -9999;
  _parent_id = -9999;
  _r_vtx = -9999;
  _phi_vtx = -9999;
  _px = -9999;
  _py = -9999;
  _pz = -9999;
  _vtx = -9999;
}

PrimaryParticle::PrimaryParticle(){
  TrueParticle();
  _key = -9999;
  _event_track = -9999;
  _true_track = -9999;
  _sub_event_track = -9999;
  _sub_event = -9999;
}

PrimaryParticle::PrimaryParticle(int id,int parent_id,float vertex,float px,float py,float pz):
TrueParticle(id,parent_id,vertex,px,py,pz)
{
   _key = -9999;
  _event_track = -9999;
  _true_track = -9999;
  _sub_event_track = -9999;
  _sub_event = -9999;
}

PrimaryPi0::PrimaryPi0(){
  PrimaryParticle();
  _dpart_list.clear();
}

PrimaryPi0::PrimaryPi0(int id,int parent_id,float vertex,float px,float py,float pz)
:PrimaryParticle(id,parent_id,vertex,px,py,pz)
{
//don't put here, or it means create a PrimaryParticle object
//  PrimaryParticle(id,parent_id,vertex,px,py,pz);
  _dpart_list.clear();
}

PrimaryPi0::~PrimaryPi0(){
  for(unsigned int i=0;i<_dpart_list.size();i++){
    if(_dpart_list[i]) delete _dpart_list[i];
    _dpart_list[i] = NULL;
  }
  _dpart_list.clear();
}


PrimPartList::PrimPartList(){
  _prim_parts.clear();
}

void PrimPartList::Reset(){
  for(unsigned int i=0;i<_prim_parts.size();i++){
    delete _prim_parts[i];
    _prim_parts[i] = NULL;
  }
  _prim_parts.clear();
}

PrimPartList::~PrimPartList(){
  Reset();
}

void PrimPartList::Clear(Option_t* option){
  Reset();
}

void PrimPartList::AddFromPrimary(PHCompositeNode* topNode){
  if(!topNode){
    std::cout<<"Null Node !!!"<<std::endl;
    return;
  }
  
  //for simnode, it may not have primary vertex
  PHGlobal* phglobal = findNode::getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = findNode::getClass<BbcOut>(topNode,"BbcOut");

  primaryWrapper* primary = findNode::getClass<primaryWrapper>(topNode, "primary");
  if(!primary){
    std::cout<<"No primaryWrapper !"<<std::endl;
    return;
  }


  //check for embeding
  Fun4AllServer *se = Fun4AllServer::instance();
  PHCompositeNode* simNode = se->topNode("SIM");
  
  if(simNode){
    PHGlobal* phglobal_sim = findNode::getClass<PHGlobal>(simNode,"PHGlobal");
    BbcOut* bbcout_sim = findNode::getClass<BbcOut>(simNode,"BbcOut");

    primaryWrapper* primary_sim = findNode::getClass<primaryWrapper>(simNode, "primary");
    

    if(phglobal_sim && bbcout_sim && primary_sim){
      phglobal = phglobal_sim;
      bbcout = bbcout_sim;
      primary = primary_sim;
    }
  }


  double vertex = -9999;
  if(!bbcout && !phglobal){
    std::cout <<"No BbcOut or PHGlobal !!!"<<std::endl;
  }
  else{
    vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
  }
  

  
  //only interested in PrimaryPi0
  std::map<int,PrimaryPi0*> prim_pi0_map;

  size_t nprim = primary->RowCount();

  for(size_t iprim=0;iprim < nprim;iprim++){
    int idparticle = primary->get_idpart(iprim);
    double px = (-1)*primary->get_px_momentum(iprim);
    double py = primary->get_py_momentum(iprim);
    double pz = primary->get_pz_momentum(iprim);
    int key = primary->get_key(iprim);
    int evt_track = primary->get_event_track(iprim);
    int sub_evt_track = primary->get_subevent_track(iprim);
    int true_track = primary->get_true_track(iprim);
    int sub_evt = primary->get_subevent(iprim);

    //no parent id, set to 0
    PrimaryParticle* prim_part = new PrimaryParticle(idparticle,0,vertex,px,py,pz);
    prim_part->SetKey(key); 
    prim_part->SetEventTrack(evt_track);
    prim_part->SetSubEventTrack(sub_evt_track);
    prim_part->SetTrueTrack(true_track);
    prim_part->SetSubEvent(sub_evt);

    _prim_parts.push_back(prim_part);
  }
}

void PrimPartList::AddFromFkin(PHCompositeNode* topNode){
  if(!topNode){
    std::cout<<"Null Node !!!"<<std::endl;
    return;
  }
  
  //for simnode, it may not have primary vertex
  PHGlobal* phglobal = findNode::getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = findNode::getClass<BbcOut>(topNode,"BbcOut");

  fkinWrapper* fkinNode = findNode::getClass<fkinWrapper>(topNode,"fkin");
  if(!fkinNode){
    std::cout<<"No fkinNode !"<<std::endl;
    return;
  }


  //check for embeding
  Fun4AllServer *se = Fun4AllServer::instance();
  PHCompositeNode* simNode = se->topNode("SIM");
  
  if(simNode){
    PHGlobal* phglobal_sim = findNode::getClass<PHGlobal>(simNode,"PHGlobal");
    BbcOut* bbcout_sim = findNode::getClass<BbcOut>(simNode,"BbcOut");

    fkinWrapper* fkinNode_sim = findNode::getClass<fkinWrapper>(simNode,"fkin");

    if(phglobal_sim && bbcout_sim && fkinNode_sim){
      phglobal = phglobal_sim;
      bbcout = bbcout_sim;
      fkinNode = fkinNode_sim;
    }
  }


  double vertex = -9999;
  if(!bbcout && !phglobal){
    std::cout <<"No BbcOut or PHGlobal !!!"<<std::endl;
  }
  else{
    vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
  }
  
  //only interested in PrimaryPi0
  std::map<int,PrimaryPi0*> prim_pi0_map;

  //grep decayed particle from pi0
  TABLE_HEAD_ST fkin_h = fkinNode->TableHeader();
  FKIN_ST *fkin = fkinNode->TableData();

  double DEG_TO_RAD = 3.141593/180;
  
  //primary particle the parent true track id is 0 
  //first loop get primary particle
  for(int fkin_index=0;fkin_index<fkin_h.nok;fkin_index++){
    FKIN_ST& kin_hit(fkin[fkin_index]);
    int parent_true_track_id = kin_hit.itparent;
    if(parent_true_track_id==0){
      int idparent = kin_hit.idparent;
      int id = kin_hit.idpart;
      int true_track_id = kin_hit.true_track;

      double px = -kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*cos( DEG_TO_RAD*kin_hit.pphi );
      double py = kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*sin( DEG_TO_RAD*kin_hit.pphi );
      double pz = kin_hit.ptot*cos( DEG_TO_RAD*kin_hit.pthet );
     
      if(id!=7){
        PrimaryParticle* prim_part = new PrimaryParticle(id,idparent,vertex,px,py,pz);
        prim_part->SetTrueTrack(true_track_id);
        _prim_parts.push_back(prim_part);
      }
      else{
        PrimaryPi0* prim_part = new PrimaryPi0(id,idparent,vertex,px,py,pz);
        prim_part->SetTrueTrack(true_track_id);
//	std::cout<<"pi0 id "<<prim_part->GetID()<<std::endl;
        _prim_parts.push_back((PrimaryParticle*)prim_part);
        prim_pi0_map.insert(std::pair<int,PrimaryPi0*>(true_track_id,prim_part)); 
      }
    }
  }
  
  
  
  for(int fkin_index=0;fkin_index<fkin_h.nok;fkin_index++){
    FKIN_ST& kin_hit(fkin[fkin_index]);
    int parent_true_track_id = kin_hit.itparent;
    
    if(prim_pi0_map.find(parent_true_track_id)==prim_pi0_map.end()) continue;

    PrimaryPi0* pm_pi0 = prim_pi0_map[parent_true_track_id];
    DecayParticle* dc_part = new DecayParticle();
    int idparent = kin_hit.idparent;
    int id = kin_hit.idpart;
    double px = -kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*cos( DEG_TO_RAD*kin_hit.pphi );
    double py = kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*sin( DEG_TO_RAD*kin_hit.pphi );
    double pz = kin_hit.ptot*cos( DEG_TO_RAD*kin_hit.pthet );
    
    int true_track_id = kin_hit.true_track;

    dc_part->SetTrackID(true_track_id);
    dc_part->SetParentTrackID(parent_true_track_id);
    dc_part->SetID(id);
    dc_part->SetParentID(idparent);
    dc_part->SetRVtx(kin_hit.r_vertex);
    dc_part->SetThetaVtx(kin_hit.th_vertx);
    dc_part->SetPhiVtx(kin_hit.ph_vertx);
    dc_part->SetPx(px);
    dc_part->SetPy(py);
    dc_part->SetPz(pz);
    dc_part->SetVertex(vertex);

    pm_pi0->Insert(dc_part); 
  }
  
}
