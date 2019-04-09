#ifndef __EXSIMEVENTV3_H__
#define __EXSIMEVENTV3_H__

#include "ExSimEventV2.h"
#include "ExHitList.h"
#include "ExPrimParticle.h"

class ExSimEventV3: public ExSimEventV2,public ExHitList
{
  private:

  public:
    ExSimEventV3();
    virtual ~ExSimEventV3();
    virtual void Clear(Option_t* option="");
    virtual void Reset();

  ClassDef(ExSimEventV3,1)
};

#endif
