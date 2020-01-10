#include "MDense.h"
#include <iostream>
#include <stdlib.h>
#include "MTensor.h"

MDense::MDense(const MShape& shape,bool set_sparse){
  _mat = new MTensor(shape,set_sparse);
  _bias.clear();
  _shape = shape;
  _set_sparse = set_sparse;

  if(shape.size()!=2){
    std::cout<<"MDense.cxx:: "<<WHERE<<" dimension not match "<<std::endl;
    exit(1);
  }
  _volume = _shape[0]*_shape[1];

  for(unsigned int i=0;i<_shape[0];i++){
    _bias.push_back(0.);
  } 
}

MDense::~MDense(){
  if(_mat){
    delete _mat;
    _mat = 0;
  }
}

MTensor* MDense::GetOutPut(MTensor* t,bool set_sparse){
  if(t->GetNDim()!=1){
    std::cout<<"MDense.cxx:: "<<WHERE<<" the dimension should be one !"<<std::endl;
    return 0;
  }
  
  if(t->GetShape(0)!=_shape[1]){
    std::cout<<"MDense.cxx:: "<<WHERE<<" dimension not match !"<<std::endl;
    return 0;
  }

  //the row number of the shape equal to the size of the output shape
  MShape out_shape(1,_shape[0]);
  MTensor* output = new MTensor(out_shape,set_sparse);
  MIndex index(2,0);
  for(unsigned int i=0;i<_shape[0];i++){
    double sum=0;
    for(unsigned int j=0;j<_shape[1];j++){
      index[0]=i;
      index[1]=j;
      sum+=(*_mat)[index]*(*t)[j];
    }
    sum+=_bias[i];
    (*output)[i]=sum;
  }
  return output;
}

void MDense::Print(){
  std::cout<<"Dense Layer"<<std::endl;
  std::cout<<"Shape: ";
  PrintShape(_shape);
  std::cout<<"Sparse "<<_set_sparse<<std::endl;
}

void MDense::SetWeights(const double* weights){
  for(unsigned int i=0;i<_volume;i++){
    (*_mat)[i]=weights[i];
  }
}

void MDense::SetWeights(const std::vector<double>& weights){
  if(weights.size()!=_volume){
    std::cout<<"MDense.cxx:: "<<WHERE<<"dimension does not match !"<<std::endl;
    return;
  }
  
  for(unsigned int i=0;i<_volume;i++){
    (*_mat)[i]=weights[i];
  }
}

void MDense::SetBias(const double* bias){
  if(!bias){
    std::cout<<"MDense.cxx "<<WHERE<<" Null Pointer"<<std::endl;
    return;
  }
  for(unsigned int i=0;i<_shape[0];i++){
    _bias[i]=bias[i];
  }
}

void MDense::SetBias(const std::vector<double>& bias ){
  if(bias.size()!=_shape[0]){
    std::cout<<"MDense.cxx:: "<<WHERE<<" Bias Size does not match "<<std::endl;
    return;
  }
  for(unsigned int i=0;i<_shape[0];i++){
    _bias[i]=bias[i];
  }
}
