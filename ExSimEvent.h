#ifndef __EXSIMEVENT_H__
#define __EXSIMEVENT_H__

#include "ExEvent.h"
#include <TObject.h>
#include <math.h>


//add true info 
//how to access Primary particle
//hits in detector: 
//https://www.phenix.bnl.gov/WWW/offline/wikioff/index.php/ANCESTRY
//particle ancestry in pisa:
//https://www.phenix.bnl.gov/WWW/offline/wikioff/index.php/Particle_ancestry_in_pisa
//



class PHCompositeNode;

class TrueParticle : public TObject
{
  private:
    //the id of particle
    int _id;
    //parent id
    int _parent_id;
    
    float _vertex;
    float _px;
    float _py;
    float _pz;
  
  public:
    TrueParticle();
    TrueParticle(int id,int parent_id,float vertex,float px,float py,float pz);
    virtual ~TrueParticle(){}
    
    int GetID() {return _id;}
    int GetParentID() {return _parent_id;}
    //don't use the vertex here for primary 
    //it may not exist
    float GetVertex() {return _vertex;}
    float GetPx() {return _px;}
    float GetPy() {return _py;}
    float GetPt() {return sqrt(_px*_px+_py*_py);}
    float GetPz() {return _pz;}

    void SetID(int id){_id = id;}
    void SetParentID(int id){_parent_id=id;}
    void SetVertex(float vtx){_vertex = vtx;}
    void SetPx(float px){_px = px;}
    void SetPy(float py){_py = py;}
    void SetPz(float pz){_pz = pz;}
  //no ";"   
  ClassDef(TrueParticle,1)
};


//add true pi0 info:
//loop particles in fkin and
//select the pi0 when its parent is
//0(primary particle), as well as
//photons decay from the pi0
//will use the vertex to match photon
//and momentum to match the pi0
//Note: the nfiles and subevent in fkin
//node are useless (they seems always 1 and 0)


class TruePi0 : public TrueParticle
{
  private:
    //add gamma info
    float _gamma_vertex[2];
    float _gamma_px[2];
    float _gamma_py[2];
    float _gamma_pz[2];
  public:
    TruePi0();
    TruePi0(int id,int parent_id,float vertex,float px,float py,float pz);
    virtual ~TruePi0(){}
    
    float GetGammaPx(unsigned int l=0) const {if(l<=1) return _gamma_px[l];else return -9999;}
    float GetGammaPy(unsigned int l=0) const {if(l<=1) return _gamma_py[l];else return -9999;}
    float GetGammaPz(unsigned int l=0) const {if(l<=1) return _gamma_pz[l];else return -9999;}
    float GetGammaVertex(unsigned int l=0) const {if(l<=1) return _gamma_vertex[l];else return -9999;}

    void SetGammaPx(float px,unsigned int l){if(l<=1) _gamma_px[l] = px;}
    void SetGammaPy(float py,unsigned int l){if(l<=1) _gamma_py[l] = py;}
    void SetGammaPz(float pz,unsigned int l){if(l<=1) _gamma_pz[l] = pz;}
    void SetGammaVertex(float vtx,unsigned int l) {if(l<=1) _gamma_vertex[l] = vtx;}
  
  ClassDef(TruePi0,1)
};

class ExSimEvent : public ExEvent
{
  private:
    std::vector<TrueParticle*> _true_particles;
  public:
    ExSimEvent();
    virtual ~ExSimEvent();
    unsigned int GetNTrue() {return _true_particles.size();}
    TrueParticle* GetTureParticle(unsigned int i) {if(i<GetNTrue()) return _true_particles[i];else return NULL;}
    void AddTureParticle(TrueParticle* particle){if(!particle)return;else _true_particles.push_back(particle);}
    //usually record pi0 at primary for embed (no decayed photons),TruePartcle
    void AddFromPrimary(PHCompositeNode*);
    //usuall record not only pi0 and also its decayed photons,
    void AddPi0FromFkin(PHCompositeNode*);
    virtual void Clear(Option_t* option="");
    virtual void Reset();
   
   //no ";"
   ClassDef(ExSimEvent,1)    
};

#endif
