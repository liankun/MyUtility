#include "ExHitList.h"
#include <TMpcExHitContainer.h>
#include <iostream>


void ExHitList::Reset(){ 
  for(unsigned int iarm=0;iarm<2;iarm++){ 
    for(unsigned int ihit=0;ihit<GetEvtNhits(iarm);ihit++ ){
      if(_ex_hit_list[iarm][ihit]) delete _ex_hit_list[iarm][ihit];
    }
    _ex_hit_list[iarm].clear();
  }
}

ExHitList::~ExHitList(){ 
  Reset();
}

ExHit* ExHitList::GetEvtHit(unsigned int i,unsigned int arm){ 
  if(arm>1) return NULL;
  if(i<_ex_hit_list[arm].size()){ 
    return _ex_hit_list[arm][i];
  }
  return NULL;
}

void ExHitList::SetEvtExHits(TMpcExHitContainer* hits,unsigned int iarm){ 
  if(!hits){
    std::cout<<"Null hits !!!"<<std::endl;
    return;
  }
  //reset first important, this method will be 
  //inherited, if there if no ExHitList, the reset
  //method will come from its child
  ExHitList::Reset();
  for(unsigned int i=0;i<hits->size();i++){ 
    TMpcExHit* ht = hits->getHit(i);
    unsigned int arm = ht->arm();
    if(arm<0 || arm>=2){ 
      std::cout<<"Bad Arm !!!"<<std::endl;
      return;
    }
    
    if(arm!=iarm && iarm!=2) continue; 

    if(ht->isGoodCombinedHit()){ 
      ExHit* ex_ht = new ExHit(ht->key(),ht->combined());
      _ex_hit_list[arm].push_back(ex_ht);
    }
  }
}
