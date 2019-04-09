#include "ExSimEventV3.h"

ExSimEventV3::ExSimEventV3(){
  ExSimEventV2();
  ExHitList();
}

void ExSimEventV3::Clear(Option_t* option){
  Reset();
}

ExSimEventV3::~ExSimEventV3(){
  Reset();
}

void ExSimEventV3::Reset(){
  ExSimEventV2::Reset();
  ExHitList::Reset();
}
