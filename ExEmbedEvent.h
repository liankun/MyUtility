#ifndef __EXEMBEDEVENT_H__
#define __EXEMBEDEVENT_H__

#include <vector>
#include "ExEventV1.h"
#include "ExPrimParticle.h"

class ExEmbedEvent : public ExEventV1, public PrimPartList 
{
  private:
  
  public:
    ExEmbedEvent();
    virtual ~ExEmbedEvent();
    virtual void Clear(Option_t* option="");
    virtual void Reset();
  
  ClassDef(ExEmbedEvent,1)
};

#endif
