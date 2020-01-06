#include "MTensor.h"
#include <stdlib.h>
#include <iostream>

MTensor::MTensor(const MShape& shape,bool set_sparse){
//  std::cout<<"MTensor Init"<<std::endl;
  _shape = shape;
  _is_sparse = set_sparse;
  int n_dim = _shape.size();
  
  if(!IsShapeValid(shape)){
    std::cout<<"MTensor:: "<<WHERE<<" invalid Shape"<<std::endl;
    _tensor=NULL;
    exit(1);
  }
//  std::cout<<"calculate volume !"<<std::endl;
  unsigned int nsize=1;
  for(int i=0;i<n_dim;i++){
    nsize*=_shape[i];
    _tf.push_back(nsize);
  }
  
//  std::cout<<"Number of the volume: "<<nsize<<std::endl;

  _volume = nsize;

  if(_is_sparse){
    _sparse_map.clear();
    _tensor = NULL;
    _nsize=0;
  }
  else{
    _tensor = new float[nsize]();
    _sparse_map.clear();
    _nsize=nsize;
  }
}

bool MTensor::IsIndexValid(const MIndex& index){
  //check if the index is valid
  if(index.size()!=_shape.size()) return false;
  for(unsigned int i=0;i<index.size();i++){
  if(index[i]>=_shape[i]){
    std::cout<<"dim "<<i<<" input index: "<<index[i]<<" tensor shape: "<<_shape[i]<<std::endl;
    return false;
  }
    if(index[i]<0) return false;
  }
  return true;
}

bool MTensor::IsShapeValid(const MShape& shape){
//  std::cout<<"check shape !"<<std::endl;
  if(shape.size()<=0) return false;
  for(unsigned int i=0;i<shape.size();i++){
    if(shape[i]<=0) return false;
  }
  return true;
}

int MTensor::Get1DIndex(const MIndex& index){
  int one_d_i = 0;
  int mul_val = 1;
  
  one_d_i+=index[0];
  for(unsigned int i=1;i<index.size();i++){
    mul_val*=_shape[i-1];
    one_d_i+=mul_val*index[i];
  }
  return one_d_i;
}

float MTensor::GetValue(const MIndex index) {
   if(!IsIndexValid(index)){
    std::cout<<"MTensor::"<<WHERE<<" Index is not valid !!!"<<std::endl;
    return -9999;
  }
  
  int one_d_i = Get1DIndex(index);
  if(_is_sparse){
    //calculate the index
    //value not exist
    if(_sparse_map.find(one_d_i)==_sparse_map.end()){
      return 0;
    }
    return _sparse_map[one_d_i];
  }
  return _tensor[one_d_i];
}

float MTensor::GetValue(const unsigned int i){
  //volume is the total size, not the effective
  //size
  if(i>=_volume){
    std::cout<<"MTensor:: "<<WHERE<<" invalid index!"<<std::endl;
    return 0;
  }

  if(_is_sparse){
    if(_sparse_map.find(i)==_sparse_map.end()){
      return 0;
    }
    return _sparse_map[i];
  }

  return _tensor[i];
}

float &MTensor::operator[](const MIndex index){
  if(!IsIndexValid(index)){
    std::cout<<"MTensor:: "<<WHERE<<" Index is not valid !!!"<<std::endl;
    exit(1);
  }

  int one_d_i = Get1DIndex(index);
  if(_is_sparse){
    //calculate the index
    //value not exist
    if(_sparse_map.find(one_d_i)==_sparse_map.end()){
      _sparse_map[one_d_i]=0;
      _nsize++;
    }
    return _sparse_map[one_d_i];
  }
  return _tensor[one_d_i];
}

float &MTensor::operator[](const unsigned int i){
  if(i>=_volume){
    std::cout<<"MTensor:: "<<WHERE<<" invalid index!"<<std::endl;
    exit(1);
  }
 
  if(_is_sparse){
    //calculate the index
    //value not exist
    if(_sparse_map.find(i)==_sparse_map.end()){
      _sparse_map[i]=0;
      _nsize++;
    }
    return _sparse_map[i];
  }

  return _tensor[i];
}

MTensor::~MTensor(){
  if(_tensor) delete[] _tensor;
  _sparse_map.clear();
}

void MTensor::Print1DTensor(){
  std::cout<<"Is Sparse: "<<_is_sparse<<std::endl;
  std::cout<<"1D Tensor Array:"<<std::endl;
  if(_is_sparse){
    for(MSparseMap::iterator it=_sparse_map.begin();it!=_sparse_map.end();it++){
      std::cout<<"1D index: "<<it->first<<" "
               <<"Value: "<<it->second<<std::endl;
    }
  }
  else{
    for(unsigned int i=0;i<_nsize;i++){
      std::cout<<"1D index: "<<i<<" "
               <<"Value: "<<_tensor[i]<<std::endl;
    }
  }
}

MIndex MTensor::GetIndexFrom1D(unsigned int idx){
  if(idx>=_volume){
    std::cout<<"MTensor.cxx:: "<<WHERE<<" the index is larger than the volume of the tensor"<<std::endl;
    return MIndex();
  }  
  
  MIndex out_index(_shape.size(),0);
  if(_shape.size()==1){
    out_index[0] = idx;
    return out_index;
  }
  for(unsigned int i=1;i<_shape.size();i++){
    unsigned int j=_shape.size()-i;
    unsigned int t_idx = idx/_tf[j-1];
    out_index[j] = t_idx;
    idx = idx - t_idx*_tf[j-1];
  }
  out_index[0] = idx;

  return out_index;
}

void MTensor::SetValue(float val){
  //set same value for each element
  for(unsigned int i=0;i<_volume;i++){
    if(_is_sparse){
      _sparse_map[i]=val;
    }
    else _tensor[i]=val;	  
  }
}

void MTensor::SetValue(){
  //set different values
  for(unsigned int i=0;i<_volume;i++){
    if(_is_sparse){
      _sparse_map[i]=i;
    }
    else _tensor[i]=i;
  }
}

void MTensor::Set1DValues(const float* values){
  if(!values){
    std::cout<<"MTensor.cxx "<<WHERE<<" null pointer !"<<std::endl;
    return;
  }

  for(unsigned int i=0;i<_volume;i++){
    //we have sparse matrix setting
    (*this)[i]=values[i];    
  }
}

void MTensor::Set1DValues(const std::vector<float>& values){
  if(values.size()!=_volume){
    std::cout<<"MTensor.cxx:: "<<WHERE<<" size not match!"<<std::endl;
    return;
  }
  for(unsigned int i=0;i<values.size();i++){
    //we have sparse matrix setting
    (*this)[i] = values[i];    
  }
}

void MTensor::Set1DValues(std::vector<float>::const_iterator begin,
		std::vector<float>::const_iterator end){
  unsigned int i=0;
  for(;begin!=end;begin++){
//    std::cout<<"1D index: "<<i<<std::endl;
    (*this)[i] = *begin;
//    std::cout<<(*this)[i]<<std::endl;
    i++;
  }
}

