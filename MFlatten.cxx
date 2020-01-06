#include "MFlatten.h"
#include "MTensor.h"
#include <iostream>

void MFlatten::Print(){
  std::cout<<"Flatten Layer !"<<std::endl;
}

MTensor* MFlatten::GetOutPut(MTensor* tensor,bool set_sparse){
  unsigned int volume = tensor->GetVolume();
  //create a shape of 1D
  MShape shape(1,volume);
  MTensor* out_tensor = new MTensor(shape);
  for(unsigned int i=0;i<volume;i++){
    (*out_tensor)[i] = tensor->GetValue(i);
  }

  return out_tensor;
}
