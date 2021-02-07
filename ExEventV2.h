#ifndef __EXEVENTV2_H__
#define __EXEVENTV2_H__

//add event header information
#include <MpcExConstants.h>
#include "ExEventV1.h"
#include <string>

class MpcExEventHeader;

class ExEventV2 : public ExEventV1{
  private:
    char _c_trig[32];
  public:
    //strigger in string
    void SetStrTrigger(unsigned int i){if(i<32) _c_trig[i]='1';}
    bool GetStrTrigger(unsigned int i){if(i<32) return _c_trig[i]=='1';return false;}
   
    ExEventV2();
    virtual ~ExEventV2();
    virtual void Clear(Option_t *option="");
    virtual void Reset();
   
  ClassDef(ExEventV2,1)
};

#endif
