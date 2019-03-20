#include "ExPrimParticle.h"
#include <PHCompositeNode.h>
#include <getClass.h>
#include <primaryWrapper.h>
#include <BbcOut.h>
#include <PHGlobal.h>

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

void PrimPartList::AddFromPrimary(PHCompositeNode* simNode){
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
