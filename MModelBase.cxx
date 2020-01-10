#include "MModelBase.h"
#include <iostream>
#include <TMpcExShower.h>
#include "ExEvent.h"


MModelBase::MModelBase(){

}

MModelBase::~MModelBase(){

}

std::vector<double> MModelBase::GetProb(TMpcExShower* shower){
  std::cout<<"MModelBase:: GetProb"<<std::endl;
  std::vector<double> result;
  return result; 
}

std::vector<double> MModelBase::GetProb(ExShower* shower){
  std::cout<<"MModelBase:: GetProb"<<std::endl;
  std::vector<double> result;
  return result; 
}

void MModelBase::Print(){
  std::cout<<"MModelBase:: Print()"<<std::endl;
}

