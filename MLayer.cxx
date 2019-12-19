#include "MLayer.h"
#include <iostream>

MLayer::~MLayer(){}

MTensor* MLayer::GetOutPut(MTensor* tensor,bool set_sparse){
  std::cout<<"Implement GetOutPut"<<std::endl;
  return 0;
}

void MLayer::Print(){
  std::cout<<"BaseLayer Information"<<std::endl;
}
