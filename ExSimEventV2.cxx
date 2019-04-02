#include "ExSimEventV2.h"

ExSimEventV2::ExSimEventV2(){ 
  ExSimEventV1();
  PrimPartList();
}

void ExSimEventV2::Clear(Option_t* option){ 
  Reset();
}

ExSimEventV2::~ExSimEventV2(){ 
  Reset();
}

void ExSimEventV2::Reset(){ 
  ExSimEventV1::Reset();
  PrimPartList::Reset();
}
