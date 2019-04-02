#ifndef __EXSIMEVENTV2_H__
#define __EXSIMEVENTV2_H__

#include "ExSimEventV1.h"
#include "ExPrimParticle.h"

class ExSimEventV2: public ExSimEventV1,public PrimPartList
{ 
  private:

  public:
    ExSimEventV2();
    virtual ~ExSimEventV2();
    virtual void Clear(Option_t* option="");
    virtual void Reset();

  ClassDef(ExSimEventV2,1)

};


#endif
