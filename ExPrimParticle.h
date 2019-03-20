#ifndef __EXPRIMPARTICLE_H__
#define __EXPRIMPARTICLE_H__

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

    PrimPartList();
    virtual ~PrimPartList();
    virtual void Clear(Option_t* option="");
    virtual void Reset();

  ClassDef(PrimPartList,1) 
};
#endif
