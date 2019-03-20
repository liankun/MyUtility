#ifndef __EXEVENTV1_H__
#define __EXEVENTV1_H__

//add event header information
#include <MpcExConstants.h>
#include "ExEvent.h"

class MpcExEventHeader;

class ExEventV1 : public ExEvent{
  private:
    int _statephase[MpcExConstants::NARMS][MpcExConstants::NPACKETS_PER_ARM];
    int _stack;
    int _parsttime[MpcExConstants::NARMS][MpcExConstants::NPACKETS_PER_ARM];

  public: 
    
    void SetMpcExEventHeader(MpcExEventHeader* ex_header);
    
    void SetStatePhase(unsigned int arm,unsigned int packet,int val){
       if(arm<MpcExConstants::NARMS && packet<MpcExConstants::NPACKETS_PER_ARM){
         _statephase[arm][packet] = val;
       }
    }

    void SetStack(int val){
      _stack = val;
    }

    void SetParstTime(unsigned int arm,unsigned int packet,int val){
      if(arm<MpcExConstants::NARMS && packet<MpcExConstants::NPACKETS_PER_ARM){
         _parsttime[arm][packet] = val;
       }
    }
    
    int GetStatePhase(unsigned int arm,unsigned int packet) const {
      if(arm<MpcExConstants::NARMS && packet<MpcExConstants::NPACKETS_PER_ARM){
        return _statephase[arm][packet];
      }
      return -9999;
    }

    int GetStack() const {
      return _stack;
    }

    int GetParstTime(unsigned int arm,unsigned int packet){
       if(arm<MpcExConstants::NARMS && packet<MpcExConstants::NPACKETS_PER_ARM){
         return _parsttime[arm][packet];
       }
       return -9999;
    }
   
    ExEventV1();
    virtual ~ExEventV1();
    virtual void Clear(Option_t *option="");
    virtual void Reset();
   
  ClassDef(ExEventV1,1)
};
#endif
