#include "ExHitList.h"
#include <TMpcExHitContainer.h>
#include <iostream>

ExHitList::ExHitList(TMpcExHitContainer* hits){ 
  SetExHits(hits);
}

void ExHitList::Reset(){ 
  for(unsigned int iarm=0;iarm<2;iarm++){ 
    for(unsigned int ihit=0;ihit<GetNhits(iarm);ihit++ ){
      if(_ex_hit_list[iarm][ihit]) delete _ex_hit_list[iarm][ihit];
    }
    _ex_hit_list[iarm].clear();
  }
}

ExHitList::~ExHitList(){ 
  Reset();
}

ExHit* ExHitList::GetHit(unsigned int i,unsigned int arm){ 
  if(arm>1) return NULL;
  if(i<GetNhits(arm)){ 
    return _ex_hit_list[arm][i];
  }
  return NULL;
}

void ExHitList::SetExHits(TMpcExHitContainer* hits){ 
  if(!hits){
    std::cout<<"Null hits !!!"<<std::endl;
    return;
  }
  //reset first
  Reset();
  for(unsigned int i=0;i<hits->size();i++){ 
    TMpcExHit* ht = hits->getHit(i);
    int arm = ht->arm();
    if(arm<0 || arm>=2){ 
      std::cout<<"Bad Arm !!!"<<std::endl;
      return;
    }
   
    if(ht->isGoodCombinedHit()){ 
      ExHit* ex_ht = new ExHit(ht->key(),ht->combined());
      _ex_hit_list[arm].push_back(ex_ht);
    }
  }
}
