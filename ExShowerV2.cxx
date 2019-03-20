#include "ExShowerV2.h"
#include <iostream>
#include <TMpcExShower.h>
#include <map>

ClassImp(ExShowerV2);

ExShowerV2::ExShowerV2(){
  InitShower();
}

ExShowerV2::~ExShowerV2(){
  ResetExShower();
}

void ExShowerV2::ResetExShower(){
  ExShowerV1::ResetExShower();
  //do ExShowerV2 Reset
  _contributors.clear();
}

void ExShowerV2::Clear(Option_t* option){
  ResetExShower();
}

void ExShowerV2::InitShower(){
  ExShowerV1::InitShower();
  //do ExShowerV2 Init
  _contributors.clear();
}

void ExShowerV2::SetContributors(TMpcExShower* shower){
  int ncontrib = shower->get_num_contributors(); 
  for(int idom=0;idom<ncontrib;idom++){
    bool t_valid = false;
    std::map<int,float>::iterator it = shower->get_contributor_by_dominance(idom+1,t_valid);
    if(t_valid){
      int ntrue = it->first;
      double efac = it->second;
      _contributors.push_back(Contributor(ntrue,efac));
    }
  }
}
