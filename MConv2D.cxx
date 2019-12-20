#include "MConv2D.h"
#include <string>
#include <iostream>
#include <stdlib.h>

MConv2D::MConv2D(const MShape& shape,
                 unsigned int nft,
		 unsigned int stride,
		 bool same_pad,
		 bool set_sparse,
		 bool for_test,
		 float fill_value){
  MLayer();
  _shape = shape;
  _nft = nft;
  _same_pad = same_pad;
  _stride = stride;

  if(_stride==0){
    std::cout<<"MConv2D:: "<<WHERE<<" invalid stride will set to 1!"<<std::endl;
    _stride=1;
  }

  for(unsigned int i=0;i<_nft;i++){
    MTensor* t_ft = new MTensor(shape,set_sparse);
    if(for_test) t_ft->SetValue(fill_value);
    _filters.push_back(t_ft);
  }
}


MConv2D::~MConv2D(){
  for(unsigned int i=0;i<_filters.size();i++){
    if(_filters[i]) delete _filters[i];
    _filters[i] = 0;
  }
  _filters.clear();
}

void MConv2D::Print(){
  std::cout<<"Convolutional 2D Layer"<<std::endl;
  std::cout<<"Shape: ";
  for(unsigned int i=0;i<_shape.size();i++){
    std::cout<<_shape[i]<<" ";
  }
  std::cout<<std::endl;
  std::cout<<"Number of filters: "<<_nft<<std::endl;
}

MTensor* MConv2D::GetOutTensor(const MShape& shape,bool set_sparse){
   //they should have the same dimension
  if(shape.size()!=_shape.size()){
    std::cout<<"MConv2D:: "<<WHERE<<" Dimension not match !"<<std::endl;
    std::cout<<"Dim of MConv2D: "<<_shape.size()<<" Dim of Input: "<<shape.size()<<std::endl;
    return 0;
  }
  
  //the shape of the filter should less tan the 
  //tensor
  for(unsigned int i=0;i<_shape.size();i++){
    if(_shape[i]>shape[i]){
      std::cout<<"MConv2D:: "<<WHERE<<" the shape of the tensor is less than filter !"<<std::endl;
      return 0;
    }
  }
  
  //get the initial out put of the tensor

  MShape out_shape(_shape.size(),0);
  out_shape[_shape.size()-1] = _nft;
  for(unsigned int j=0;j<_shape.size()-1;j++){
    unsigned int sp = shape[j];
    if(!_same_pad){
      sp = (shape[j]-_shape[j])/_stride+1;
    }
    out_shape[j] = sp;
  }
 
  MTensor* out_tensor = new MTensor(out_shape,set_sparse);
  return out_tensor;
}

MTensor* MConv2D::GetOutPut(MTensor* tensor,bool set_sparse){
  //get the initial output tensor
  std::cout<<"Input Tensor Shape: "<<std::endl;
  PrintShape(tensor->GetShape());

  MTensor* out_tensor = GetOutTensor(tensor->GetShape(),set_sparse);
  if(!out_tensor) return 0;
  
  std::cout<<"Get new out tensor !"<<std::endl;
  PrintShape(out_tensor->GetShape());
  //no padding first and 
  //implementation for the common matrix multiplication
  //The last dimension is always view as the 
  //channels
  //Here is the algorithm
  //1. from the filter and input tensor shape, we can get the
  //   output tensor, which all value is set to 0
  //2. the dimension of the tensor is undefined.The multiplication
  //   is based on 1D array
  //3. we loop along the 1D array,for each value, we map back to the
  //   corresponding index of output tensor
  //4. when we get the index of output tensor, we can get the multiplication part
  //   of the input tensor
  //5. perform matrix multiplication
  //6. the key is the intertransformation between the 1D array index and multidimentional 
  //   index
  
  unsigned int out_volume = out_tensor->GetVolume();
  std::cout<<"volume of output tensor: "<<out_volume<<std::endl;
  //index used for output tensor
  for(unsigned int i=0;i<out_volume;i++){
    //get corresponding index
    MIndex out_indx = out_tensor->GetIndexFrom1D(i);
//    std::cout<<"Get index out of 1D for out tensor"<<std::endl;
    unsigned int i_ft = out_indx[out_tensor->GetNDim()-1];
    //access the corresponding filter
    MTensor* t_filter = _filters[i_ft];
    unsigned int ft_volume = t_filter->GetVolume();
//    std::cout<<"volume of filter: "<<ft_volume<<std::endl;
    //mat multiplication
    //the filter is the multidemension
    //we can not use the multiple for loop
    //we will first flat them into one array
    //then do the multiplication
    
    //get the corresponding index in the input tensor
    MIndex ref_index = MultiplyIndex(out_indx,_stride);
    //the last index is not right,shoudl be 0
    ref_index[ref_index.size()-1]=0; 

//    std::cout<<"ref index: "<<std::endl;
//    PrintIndex(ref_index);
    

    //the result of the matrix multiplication
    float val = 0;
    for(unsigned int j=0;j<ft_volume;j++){
      MIndex index0 = t_filter->GetIndexFrom1D(j);
//      std::cout<<"Get index out of 1D for filter !"<<std::endl;
      //get the corresponding element for input tensor
      MIndex index1 = AddIndex(index0,ref_index);
//      std::cout<<"Get the index for input tensor !"<<std::endl;
      float val0 = t_filter->GetValue(index0);
//      std::cout<<"Get value of filter "<<val0<<std::endl;
      float val1 = tensor->GetValue(index1);;
//      std::cout<<"Get Value of input tensor "<<val1<<std::endl;
      val+=val0*val1;
    }
    (*out_tensor)[out_indx] = val;
  }

  return out_tensor;
}

