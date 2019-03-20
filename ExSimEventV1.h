#ifndef __EXSIMEVENTV1_H__
#define __EXSIMEVENTV1_H__

#include <iostream>
#include <TObject.h>
#include <map>
#include <vector>
#include "ExEvent.h"
#include "ExSimEvent.h"


class FkinParticle : public TrueParticle
{
  private:
    int _track_id;
    int _parent_track_id;
    float _r_vtx;
    float _theta_vtx;
    float _phi_vtx;
    FkinParticle* _parent;
    std::vector<int> _n_part_depth;
    std::vector<FkinParticle*> _children;
  public:
    FkinParticle():TrueParticle(){
      _track_id = -9999;
      _parent_track_id = -9999;
      _r_vtx = -9999;
      _theta_vtx = -9999;
      _phi_vtx = -9999;
      _n_part_depth.clear();
      _children.clear();
      _parent = NULL;
    }
    FkinParticle(int id,int parent_id,float vertex,float px,float py,float pz):
    TrueParticle(id,parent_id,vertex,px,py,pz)
    {
      _track_id = -9999;
      _parent_track_id = -9999;
      _r_vtx = -9999;
      _theta_vtx = -9999;
      _phi_vtx = -9999;
      _n_part_depth.clear();
      _children.clear();
      _parent = NULL;
    }
    virtual ~FkinParticle();
      
    void SetTrackID(int val){_track_id = val;}
    void SetParentTrackID(int val){ _parent_track_id = val;}
    void SetRVtx(float val){_r_vtx = val;}
    void SetThetaVtx(float val){_theta_vtx = val;}
    void SetPhiVtx(float val){_phi_vtx = val;}
    void SetParent(FkinParticle* fk_part) {if(fk_part) _parent = fk_part;} 
    int GetNDepthParticle(unsigned int val){if(val >=0 && val <_n_part_depth.size()) return _n_part_depth[val];return -9999;}
    int GetNDepth(){return _n_part_depth.size();}
    int GetNChildren(){return _children.size();}
    FkinParticle* GetChild(unsigned int i){if(i>=0&&i<_children.size()) return _children[i];return NULL;}
    FkinParticle* GetParent(){return _parent;}
    int GetTrackID(){return _track_id;}
    int GetParentTrackID(){return _parent_track_id;}
    float GetRVtx(){return _r_vtx;}
    float GetThetaVtx(){return _theta_vtx;}
    float GetPhiVtx(){return _phi_vtx;}
    
    //print all its info
    void Print();

    std::vector<FkinParticle*>* GetChildren(){return &_children;}
    std::vector<int>* GetNPartDepth(){return &_n_part_depth;}
    
    ClassDef(FkinParticle,1) 
};


class ExSimEventV1 : public ExEvent
{
  private:
    std::map<int,FkinParticle*> _fk_map;
    //the number of particle in each decayed depth 
    //use DFS, depth is just to keep track of the 
    //depth
    const std::vector<int>* set_n_depth(FkinParticle* particle,int depth);
    //clean unwanted particle in decayed particles
    //the depth is just to keep track of depth
    //must_remove is to make sure, if parent is removed and then
    //all children should be removed
    void clean_particles(FkinParticle* particle,int depth,bool must_remove=false);
    
    //construct a fkin particle list for easy access
    std::vector<FkinParticle*> _fk_lists;
    
    //be careful: _fk_lists and _fk_map share the same pointer to
    //the FkinParticle,but when fill the tree, the object FkinParticle will 
    //be created twice. this means two times space will be used !;
       
    void set_fk_list(){
      _fk_lists.clear();
      for(iterator it = FkinBegin();it!=FkinEnd();it++){
        _fk_lists.push_back(it->second);
      }
      _is_fk_list_set = true;
    }
    float _thresh;
    bool _is_fk_list_set;
  public:
//    typedef std::pair<int,FkinParticle*> MapItem;
    
    typedef std::map<int,FkinParticle*>::iterator iterator;
    typedef std::map<int,FkinParticle*>::const_iterator const_iterator;
    ExSimEventV1();
    virtual ~ExSimEventV1();
    virtual void Clear(Option_t* option="");
    virtual void Reset();
    //Energy threshold in GeV to record that particle
    void SetThreshold(float val=0.5){_thresh = val;}
    void AddFkinParticles(PHCompositeNode*);
    
    FkinParticle* GetFkParticle(unsigned int i){
      if(!_is_fk_list_set) set_fk_list();
      if(i<_fk_lists.size()){
        return _fk_lists[i];
      }
      else return NULL;
    }
    
    int GetNFkin(){
      if(!_is_fk_list_set) set_fk_list();
      return _fk_lists.size();
    }

    FkinParticle* GetFkPartByTrackID(unsigned int i){
      if(_fk_map.find(i)!=_fk_map.end()){
       return _fk_map[i];
      }
      else return NULL;
    }
    
    iterator FkinBegin(){return _fk_map.begin();}
    iterator FkinEnd(){return _fk_map.end();}
    
    //print all its offsprings,use BFS
    void PrintFkinParticle(FkinParticle* fk_part);

  ClassDef(ExSimEventV1,1)
  
};
#endif
