#ifndef __EXPRIMPARTICLE_H__
#define __EXPRIMPARTICLE_H__

#include <TObject.h>
#include <vector>
#include "ExSimEvent.h"

class PHCompositeNode;

class PrimaryParticle : public TrueParticle
{
  private:
    int _key;
    int _event_track;
    int _true_track;
    int _sub_event_track;
    int _sub_event;

  public:
    PrimaryParticle();
    PrimaryParticle(int id,int parent_id,float vertex,float px,float py,float pz);
    virtual ~PrimaryParticle(){}

    int GetKey() const {return _key;}
    int GetEventTrack() const {return _event_track;}
    int GetTrueTrack() const {return _true_track;}
    int GetSubEventTrack() const {return _sub_event_track;}
    int GetSubEvent() const {return _sub_event;}

    void SetKey(int val) {_key = val;}
    void SetEventTrack(int val) {_event_track = val;}
    void SetTrueTrack(int val) {_true_track = val;}
    void SetSubEventTrack(int val) {_sub_event_track = val;}
    void SetSubEvent(int val) {_sub_event = val;}

  ClassDef(PrimaryParticle,1)

};

//create primary pi0 (include the decayed photons)

class DecayParticle : public TObject{
  private:
    int _track_id;
    int _parent_track_id;
    int _id;
    int _parent_id;
    float _r_vtx;
    float _phi_vtx;
    float _theta_vtx;
    float _px;
    float _py;
    float _pz;
    float _vtx;
  public:
    DecayParticle();
    virtual ~DecayParticle(){}
    
    int GetTrackID(){return _track_id;}
    int GetParentTrackID(){ return _parent_track_id;}
    int GetID() {return _id;}
    int GetParentID(){return _parent_id;}
    float GetRVtx(){return _r_vtx;}
    float GetThetaVtx(){return _theta_vtx;}
    float GetPhiVtx(){return _phi_vtx;}
    float GetPx() {return _px;}
    float GetPy() {return _py;}
    float GetPz() {return _pz;}
    float GetVertex() {return _vtx;}
   
    
    void SetTrackID(int val){_track_id = val;}
    void SetParentTrackID(int val){ _parent_track_id = val;}
    void SetID(int val) {_id = val;}
    void SetParentID(int val) {_parent_id = val;}
    void SetRVtx(float val){_r_vtx = val;}
    void SetThetaVtx(float val){_theta_vtx = val;}
    void SetPhiVtx(float val){_phi_vtx = val;}
    void SetPx(float val) {_px = val;}
    void SetPy(float val) {_py = val;}
    void SetPz(float val) {_pz = val;}
    void SetVertex(float val) {_vtx = val;}

  ClassDef(DecayParticle,1)

};

class PrimaryPi0 : public PrimaryParticle
{
  //add decaied photon
  public:
    PrimaryPi0();
    PrimaryPi0(int id,int parent_id,float vertex,float px,float py,float pz);
    virtual ~PrimaryPi0();
    
    unsigned int GetNDecayParticle() {return _dpart_list.size();}
    DecayParticle* GetDecayParticle(unsigned int i) {if(i<_dpart_list.size()) return _dpart_list[i];return NULL;}
  
    void Insert(DecayParticle* dc_part) {_dpart_list.push_back(dc_part);}
  
  private:
    std::vector<DecayParticle*> _dpart_list; 

  ClassDef(PrimaryPi0,1)

};

class PrimPartList
{
  private:
    std::vector<PrimaryParticle*> _prim_parts;
  public:
    unsigned int GetNPrim() const {return _prim_parts.size();}
    
    PrimaryParticle* GetPrim(unsigned int i) const{
      if(i<GetNPrim()) return _prim_parts[i];
      else return NULL;
    }
    
    //add all primary particle info
    void AddFromPrimary(PHCompositeNode*);
    
    //add primary from Fkin some value may be different
    //fill all primary particles
    //and fill the particle decay from pi0s and 
    void AddFromFkin(PHCompositeNode*);

    PrimPartList();
    virtual ~PrimPartList();
    virtual void Clear(Option_t* option="");
    virtual void Reset();

  ClassDef(PrimPartList,1) 
};
#endif
