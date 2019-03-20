#include "ExSimEventV1.h"
#include <iostream>
#include <PHCompositeNode.h>
#include <iostream>
#include <fkinWrapper.h>
#include <getClass.h>
#include <queue>




FkinParticle::~FkinParticle(){
  _n_part_depth.clear();
  _children.clear();
  _parent = NULL;
}

void FkinParticle::Print(){
  std::cout<<"Id "<<GetID()<<" "
           <<"Parent Id "<<GetParentID()<<" "
	   <<"Vertex: "<<GetVertex()<<" "
	   <<"Px: "<<GetPx()<<" "
	   <<"Py: "<<GetPy()<<" "
	   <<"Pz: "<<GetPz()<<" "
	   <<std::endl;
  std::cout<<"track id: "<<_track_id<<" "
           <<"parent track id: "<<_parent_track_id<<" "
	   <<"Ndepth: "<<GetNDepth()<<" "
	   <<"NChildren: "<<GetNChildren()<<" "
	   <<std::endl;
  std::cout<<"Ndepth info: ";
  for(unsigned int i=0;i<_n_part_depth.size();i++){
    std::cout<<"depth "<<i<<" "<<_n_part_depth[i]<<" ";
  }
  std::cout<<std::endl;
  std::cout<<"Childer info: ";
  for(unsigned int i=0;i<_children.size();i++){
    FkinParticle* child = _children[i];
    std::cout<<child->GetID()<<" "<<child->GetParentID()<<" ";
  }
  std::cout<<std::endl;
}

ExSimEventV1::ExSimEventV1(){
  ExEvent();
  _fk_lists.clear();
  _fk_map.clear();
  _thresh = 0.5;
  _is_fk_list_set = false;
}

void ExSimEventV1::Clear(Option_t*){
  Reset();
}

void ExSimEventV1::Reset(){
  ExEvent::Reset();
  for(iterator it = _fk_map.begin();it!=_fk_map.end();it++){
    delete it->second;
    it->second = NULL;
  }
  _fk_map.clear(); 
  _fk_lists.clear();
  _is_fk_list_set = false;
}

ExSimEventV1::~ExSimEventV1(){
  Reset();
}

void ExSimEventV1::AddFkinParticles(PHCompositeNode* simNode){
  if(!simNode){
    std::cout<<"Null Node !!!"<<std::endl;
    return;
  }

  fkinWrapper* fkinNode = findNode::getClass<fkinWrapper>(simNode,"fkin");
  if(!fkinNode){
    std::cout<<"No fkinNode !"<<std::endl;
    return;
  }
  TABLE_HEAD_ST fkin_h = fkinNode->TableHeader();
  FKIN_ST *fkin = fkinNode->TableData();
  double DEG_TO_RAD = 3.141593/180;

  
  // for the primary particle the parent track id is 0 
  // and for the track id in fkin, starts at 1
  for(int fkin_index = 0; fkin_index < fkin_h.nok; fkin_index++){
    FKIN_ST& kin_hit(fkin[fkin_index]);
    int idparent = kin_hit.idparent;
    int id = kin_hit.idpart;
    double px = -kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*cos( DEG_TO_RAD*kin_hit.pphi );
    double py = kin_hit.ptot*sin( DEG_TO_RAD*kin_hit.pthet )*sin( DEG_TO_RAD*kin_hit.pphi );
    double pz = kin_hit.ptot*cos( DEG_TO_RAD*kin_hit.pthet );
    
    int true_track_id = kin_hit.true_track;
    
    iterator it = _fk_map.find(true_track_id);
    //check if this particle exsits in fkin map
    if(it!=_fk_map.end()){
      //already exist and set the px,py,pz
      FkinParticle* fk_part = it->second;
      fk_part->SetID(id);
      fk_part->SetParentID(idparent);
      fk_part->SetTrackID(true_track_id);
      fk_part->SetParentTrackID(kin_hit.itparent);
      fk_part->SetPx(px);
      fk_part->SetPy(py);
      fk_part->SetPz(pz);
      fk_part->SetVertex(kin_hit.z_vertex);
      fk_part->SetRVtx(kin_hit.r_vertex);
      fk_part->SetThetaVtx(kin_hit.th_vertx);
      fk_part->SetPhiVtx(kin_hit.ph_vertx);
    }
    else{
      //not exist
      FkinParticle* fk_part = new FkinParticle(id,idparent,kin_hit.z_vertex,px,py,pz);
      fk_part->SetTrackID(true_track_id);
      fk_part->SetParentTrackID(kin_hit.itparent);
      fk_part->SetRVtx(kin_hit.r_vertex);
      fk_part->SetThetaVtx(kin_hit.th_vertx);
      fk_part->SetPhiVtx(kin_hit.ph_vertx);
      _fk_map.insert(std::pair<int,FkinParticle*>(kin_hit.true_track,fk_part));
    }
    //check if the parent exists
    it = _fk_map.find(kin_hit.itparent);
    if(it != _fk_map.end()){
      //exist
      FkinParticle* fk_part = it->second;
      std::vector<FkinParticle*>* children = fk_part->GetChildren(); 
      children->push_back(_fk_map[true_track_id]);
      _fk_map[true_track_id]->SetParent(fk_part);
    }
    else{
      FkinParticle* fk_part = new FkinParticle();
      fk_part->SetTrackID(kin_hit.itparent);
      fk_part->SetID(idparent);
      std::vector<FkinParticle*>* children = fk_part->GetChildren();
      children->push_back(_fk_map[kin_hit.true_track]);
      _fk_map.insert(std::pair<int,FkinParticle*>(kin_hit.itparent,fk_part));
      _fk_map[kin_hit.true_track]->SetParent(fk_part);
    }
  }

  //set ndepth for primary particle
  //0 stands for the dummy particle,
  //which is the parent of all primary
  //particles
  const_iterator it = _fk_map.find(0);
  if(it == _fk_map.end()){
    std::cout<<"No Dummy Particle !"<<std::endl;
    return;
  }

  FkinParticle* fk_part = it->second;
  set_n_depth(fk_part,0);
  //clean the particle according to the threshold 
  clean_particles(fk_part,0);
}

//only apply to first in _fk_map (which is not the particle, just the dumy parent of all
//parimay particle)
const std::vector<int>* ExSimEventV1::set_n_depth(FkinParticle* fk_part,int depth){
  if(!fk_part){
    std::cout<<"NULL FkinParticle !"<<std::endl;
    return NULL;
  }
  int nchildren = fk_part->GetNChildren();
  std::vector<int>* n_part_depth = fk_part->GetNPartDepth();
  n_part_depth->clear();
  if(nchildren <= 0) return n_part_depth;
  n_part_depth->push_back(nchildren);
  
  for(int i=0;i<nchildren;i++){
    FkinParticle* child = fk_part->GetChild(i);
    const std::vector<int>* temp_list = set_n_depth(child,depth+1);
    if(temp_list->size()==0) continue;
    for(unsigned int j=0;j<temp_list->size();j++){
      if(j+1 >= n_part_depth->size()) n_part_depth->push_back(temp_list->at(j));
      else n_part_depth->at(j+1) += temp_list->at(j);
    }
  }
  return n_part_depth;
}

//only apply to the first dummy particle (the parent of all primary particle)
void ExSimEventV1::clean_particles(FkinParticle* fk_part,int depth,bool must_remove){
  if(!fk_part){
    std::cout<<"NULL FkinParticle !"<<std::endl;
    return;
  }
  
  bool should_remove = false;
  should_remove = must_remove;

  float px = fk_part->GetPx();
  float py = fk_part->GetPy();
  float pz = fk_part->GetPz();
  float total_e = sqrt(px*px+py*py+pz*pz);
  int id = fk_part->GetID();
  
  //id == 0 is not the particle and its children are 
  //primary particle.just for easy access for primary
  //particles
  if(total_e < _thresh && id>0) should_remove = true;

  std::vector<FkinParticle*>* children = fk_part->GetChildren();

  std::vector<FkinParticle*>::iterator it = children->begin();
  while(it!=children->end()){
    FkinParticle* child = *it;
    float tmp_px = child->GetPx();
    float tmp_py = child->GetPy();
    float tmp_pz = child->GetPz();
    float tmp_tot_p = sqrt(tmp_px*tmp_px+tmp_py*tmp_py+tmp_pz*tmp_pz);
    clean_particles(child,depth+1,should_remove);
    if(tmp_tot_p < _thresh || should_remove){
      it=children->erase(it);
    }
    else it++;
  }

  if(should_remove){
    int track_id = fk_part->GetTrackID();
    delete fk_part;
    _fk_map.erase(track_id);
  }
  return;
}

void ExSimEventV1::PrintFkinParticle(FkinParticle* fk_part){
  if(!fk_part){
    std::cout<<"Null pointer !!!"<<std::endl;
    return;
  }
  // the first is the depth
  typedef std::pair<int,FkinParticle*> ITEM; 
  std::queue<ITEM> fkin_q;
  fkin_q.push(ITEM(0,fk_part));
  while(!fkin_q.empty()){
    ITEM item = fkin_q.front();
    fkin_q.pop();
    FkinParticle* particle = item.second;
    int depth = item.first;
    std::cout<<"Depth: "<<depth<<std::endl;
    particle->Print();
    int nchildren = particle->GetNChildren();
    for(int i=0;i<nchildren;i++){
      fkin_q.push(ITEM(depth+1,particle->GetChild(i)));
    }
  }
}
