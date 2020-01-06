#include "MPooling.h"
#include "MTensor.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

MPooling::MPooling(const MShape& shape,
		 unsigned int stride,
		 bool same_pad,
		 Method md
		 ){
  MLayer();
  _shape = shape;
  _same_pad = same_pad;
  _stride = stride;
  _dummy_ft = new MTensor(_shape,true);
  _method = md;

  if(_stride==0){
    std::cout<<"MPooling:: "<<WHERE<<" invalid stride will set to 2!"<<std::endl;
    _stride=2;
  }

  if(_shape.size()<=1){
    std::cout<<"MPooling:: "<<WHERE<<" the dimension is less than one !!!, may cause issue !"<<std::endl;
  }
  
}

MPooling::~MPooling(){
  if(_dummy_ft) delete _dummy_ft;
  _dummy_ft = 0;
}

void MPooling::Print(){
  std::cout<<"Pooling Layer"<<std::endl;
  std::cout<<"Shape: ";
  for(unsigned int i=0;i<_shape.size();i++){
    std::cout<<_shape[i]<<" ";
  }
  std::cout<<std::endl;
  std::cout<<"Method: ";
  if(_method==MAX) std::cout<<"MAX"<<std::endl;
  else std::cout<<"AVG"<<std::endl;
}

MTensor* MPooling::GetOutTensor(const MShape& shape,bool set_sparse){
   //the dimension of shape should one less than input tensor
  if(shape.size()!=_shape.size()+1){
    std::cout<<"MPooling:: "<<WHERE<<" Dimension not match !"<<std::endl;
    std::cout<<"MPooling:: "<<WHERE<<" Dim of MPooling: "<<_shape.size()<<" Dim of Input: "<<shape.size()<<std::endl;
    return 0;
  }
  
  //the shape of the filter should less tan the 
  //tensor
  for(unsigned int i=0;i<_shape.size();i++){
    if(_shape[i]>shape[i]){
      std::cout<<"MPooling:: "<<WHERE<<" the shape of the tensor is less than pooling !"<<std::endl;
      return 0;
    }
  }
  
  //get the initial out put of the tensor

  MShape out_shape(shape.size(),0);
  out_shape[shape.size()-1] = shape[shape.size()-1];
  for(unsigned int j=0;j<shape.size()-1;j++){
    unsigned int sp = shape[j];
    if(!_same_pad){
      if(shape[j]<_shape[j]){
        std::cout<<"MPooling:: "<<WHERE<<" bad shape for tensor"<<std::endl;
	return 0;
      }
      sp = (shape[j]-_shape[j])/_stride+1;
    }
    else{
      sp=ceil(1.0*shape[j]/_stride);
    }
    out_shape[j] = sp;
  }
 
//  std::cout<<"Output tensor shape: "<<std::endl;
//  PrintShape(out_shape);
  MTensor* out_tensor = new MTensor(out_shape,set_sparse);
  return out_tensor;
}


//without padding version
MTensor* MPooling::GetOutPut(MTensor* tensor,bool set_sparse){
  //get the initial output tensor
//  std::cout<<"Input Tensor Shape: "<<std::endl;
//  PrintShape(tensor->GetShape());

  MTensor* out_tensor = GetOutTensor(tensor->GetShape(),set_sparse);
  if(!out_tensor){
    std::cout<<"MPooling:: "<<WHERE<<" return invalid output tensor ! "<<std::endl;
    return 0;
  }
  
  if(_same_pad) SetPaddingShift(out_tensor->GetShape());

  unsigned int out_volume = out_tensor->GetVolume();
//  std::cout<<"volume of output tensor: "<<out_volume<<std::endl;
  //index used for output tensor
  for(unsigned int i=0;i<out_volume;i++){
    //get corresponding index
    MIndex out_indx = out_tensor->GetIndexFrom1D(i);
//    std::cout<<"Get index out of 1D for out tensor"<<std::endl;
        
    //get the corresponding index in the input tensor
    MIndex ref_index = MultiplyIndex(out_indx,_stride);
    //the last index should be the same as in out_index
    ref_index[ref_index.size()-1]=out_indx[out_tensor->GetNDim()-1]; 

//    std::cout<<"ref index: "<<std::endl;
//    PrintIndex(ref_index);
    
    //the result of the matrix multiplication
    float mean_val = 0;
    float max_val = 0;
    for(unsigned int j=0;j<_dummy_ft->GetVolume();j++){
      MIndex index0 = _dummy_ft->GetIndexFrom1D(j);
//      std::cout<<"Get index out of 1D for filter !"<<std::endl;
      //get the corresponding element for input tensor
      MIndex index1 = AddIndex(index0,ref_index);
//      std::cout<<"Get the index for input tensor !"<<std::endl;
        
      float val = 0;
      if(_same_pad){
	if(GetPaddingIndex(index1,tensor->GetShape())){
          val = tensor->GetValue(index1);
	}
      }
      else{
        val = tensor->GetValue(index1);
      }
      
      if(j==0){
        mean_val+=val;
	max_val=val;
      }
      else{
        mean_val+=val;
	if(max_val<val) max_val = val;
      }
    }
    float val = max_val;
    if(_method==AVG) val = mean_val/_dummy_ft->GetVolume();
    (*out_tensor)[out_indx] = val;
  }

  return out_tensor;
}

void MPooling::SetPaddingShift(const MShape &shape){
  //refer to MConv for detail 
  std::vector<unsigned int> shift_start(shape.size(),0);
  std::vector<unsigned int> shift_end(shape.size(),0);
  _padding_shift0 = shift_start;
  _padding_shift1 = shift_end;
  //setting the value of the start and end
  for(unsigned int i=0;i<shape.size()-1;i++){
    //if we use the unsigned int for math,
    //will result in troubles
    
    int m = shape[i];
    int n = _shape[i];
    int s = _stride;
    int o = ceil(1.0*m/s);
    int p = (o-1)*s+n-m;
    _padding_shift0[i]=p/2;
    _padding_shift1[i]=p-_padding_shift0[i];
  }
}

bool MPooling::GetPaddingIndex(MIndex &index,const MShape& shape){
  if(index.size()==0 || shape.size()==0) return false;
  for(unsigned int i=0;i<shape.size()-1;i++){
    if(index[i]<_padding_shift0[i]) return false;
    if(index[i]>=shape[i]+_padding_shift0[i]) return false;
    index[i] = index[i]-_padding_shift0[i];
  }
  
  return true;
}
