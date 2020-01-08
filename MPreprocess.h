#ifndef __MPREPROCESS_H__
#define __MPREPROCESS_H__

/**
 *a class for transform the shower 
 *into a trainable (65,65,8) tensor
 *for classification
 *
 *
**/

#include "MIndexing.h"
#include <map>

class MTensor;
class TMpcExShower;
class ExShower;
class TMpcExHitContainer;
class TMpcExHit;

const float MINSPACE=-20; //20cm
const float DSPACE=40./214.;//in cm
const float SHOWERSIZE=6;//a size of 6cm x 6cm
const int NSPACE=int(2*SHOWERSIZE/DSPACE)+1;

class MPreprocess{
  public:
    MPreprocess();
    MPreprocess(const TMpcExHitContainer*);
    virtual ~MPreprocess();
    
    MTensor* GetTensor(TMpcExShower* shower,bool set_spares=false);
    MTensor* GetTensor(ExShower* shower,bool set_sparse=false);

    bool SetTensor(MTensor* t,TMpcExShower* shower);
    bool SetTensor(MTensor* t,ExShower* shower);
    
    void SetHitContainer(const TMpcExHitContainer*);
    void SetShape(MShape& shape);
    
    bool SetTensorByHit(int layer,float x,float y,float e,MTensor* t);
  private:
    MShape _shape;
    std::map<int,TMpcExHit*> _ex_hit_map; 
};

#endif
