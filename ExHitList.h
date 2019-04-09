#ifndef __EXHITLIST_H__
#define __EXHITLIST_H__

#include <vector>
#include "ExEvent.h"

class TMpcExHitContainer;

class ExHitList
{ 
  private:
    //one for south and one for north
    //0:South and 1:North
    std::vector<ExHit*> _ex_hit_list[2];
  
  public:
    ExHitList(){_ex_hit_list[0].clear();_ex_hit_list[1].clear();}
    virtual ~ExHitList();
    virtual void Reset();
    unsigned int GetEvtNhits(unsigned int arm=1){if(arm<=1) return _ex_hit_list[arm].size();return -9999;}
    ExHit* GetEvtHit(unsigned int i,unsigned int arm=1);
    void SetEvtExHits(TMpcExHitContainer* hits,unsigned int arm=1);//0 south,1 north, 2: both arm
  
  //very important don't foget
  ClassDef(ExHitList,1)
};

#endif
