#include "ExShowerV1.h"
#include <iostream>

ClassImp(ExShowerV1);

ExShowerV1::ExShowerV1(){
  InitShower();  
}

ExShowerV1::~ExShowerV1(){
  ResetExShower();
}

void ExShowerV1::ResetExShower(){
  ExShower::ResetExShower();
  for(int i=0;i<5;i++){
    for(int j=0;j<5;j++){
      _quality[i][j] = -9999;
    }
  }
}

void ExShowerV1::Clear(Option_t* option){
  ResetExShower(); 
}

void ExShowerV1::InitShower(){
  ExShower::InitShower();
  for(int i=0;i<5;i++){
    for(int j=0;j<5;j++){
      _quality[i][j] = -9999;
    }
  }

}

void ExShowerV1::SetMpcPulseQuality(float quality[5][5]){
  if(!quality){
    std::cout<<"ExShowerV1::SetMpcPulseQuality NULL Arrary !!! "<<std::endl;
    return;
  }

  for(int i=0;i<5;i++){
    for(int j=0;j<5;j++){
      _quality[i][j] = quality[i][j];
    }
  }
}
