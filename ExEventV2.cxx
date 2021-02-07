#include "ExEventV2.h"
#include <iostream>

ExEventV2::ExEventV2(){
  ExEventV1();
  for(unsigned int i=0;i<32;i++){
    _c_trig[i]='0'; 
  }
}

void ExEventV2::Clear(Option_t*){
  Reset();
}

ExEventV2::~ExEventV2(){
  Reset();
}

void ExEventV2::Reset(){
  ExEventV1::Reset();
  for(unsigned int i=0;i<32;i++){
    _c_trig[i]='0'; 
  }
}

