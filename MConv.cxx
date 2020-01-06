#include "MConv.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <iterator>

MConv::MConv(const MShape& shape,
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
    std::cout<<"MConv:: "<<WHERE<<" invalid stride will set to 1!"<<std::endl;
    _stride=1;
  }

  if(_shape.size()<=1){
    std::cout<<"the dimension is less than one !!!, may cause issue !"<<std::endl;
  }

  for(unsigned int i=0;i<_nft;i++){
    MTensor* t_ft = new MTensor(shape,set_sparse);
    _ft_volume = t_ft->GetVolume();
    float b=0;
    if(for_test){
      if(fill_value>-9999) t_ft->SetValue(fill_value);
      else t_ft->SetValue();
      b=1;
    } 
    _filters.push_back(t_ft);
    _bias.push_back(b);
  }

}


MConv::~MConv(){
  for(unsigned int i=0;i<_filters.size();i++){
    if(_filters[i]) delete _filters[i];
    _filters[i] = 0;
  }
  _filters.clear();
}

void MConv::Print(){
  std::cout<<"Convolutional 2D Layer"<<std::endl;
  std::cout<<"Shape: ";
  for(unsigned int i=0;i<_shape.size();i++){
    std::cout<<_shape[i]<<" ";
  }
  std::cout<<std::endl;
  std::cout<<"Number of filters: "<<_nft<<std::endl;
}

MTensor* MConv::GetOutTensor(const MShape& shape,bool set_sparse){
   //they should have the same dimension
  if(shape.size()!=_shape.size()){
    std::cout<<"MConv:: "<<WHERE<<" Dimension not match !"<<std::endl;
    std::cout<<"Dim of MConv: "<<_shape.size()<<" Dim of Input: "<<shape.size()<<std::endl;
    return 0;
  }
  
  //the shape of the filter should less tan the 
  //tensor
  for(unsigned int i=0;i<_shape.size();i++){
    //the last dimension must be the same
    if(i==_shape.size()-1){
      if(_shape[i]!=shape[i]){
        std::cout<<"MConv:: "<<WHERE<<" the last dimension must be the same for filter ! "<<std::endl;
	return 0;
      }
    }

    if(_shape[i]>shape[i]){
      std::cout<<"MConv:: "<<WHERE<<" the shape of the tensor is less than filter !"<<std::endl;
      return 0;
    }
  }
  
  //get the initial out put of the tensor

  MShape out_shape(_shape.size(),0);
  out_shape[_shape.size()-1] = _nft;
  for(unsigned int j=0;j<_shape.size()-1;j++){
    unsigned int sp = shape[j];

    if(!_same_pad){
      if(shape[j]<_shape[j]){
        std::cout<<"MConv:: "<<WHERE<<" bad shape for tensor"<<std::endl;
	return 0;
      }
      sp = (shape[j]-_shape[j])/_stride+1;
    }
    else{
      sp=ceil(1.0*shape[j]/_stride);
    }
    out_shape[j] = sp;
  }
 
  MTensor* out_tensor = new MTensor(out_shape,set_sparse);
  return out_tensor;
}


//without padding version
MTensor* MConv::GetOutPut(MTensor* tensor,bool set_sparse){
  //get the initial output tensor
//  std::cout<<"Input Tensor Shape: "<<std::endl;
//  PrintShape(tensor->GetShape());

  MTensor* out_tensor = GetOutTensor(tensor->GetShape(),set_sparse);
  if(!out_tensor) return 0;
  
//  std::cout<<"Get new out tensor !"<<std::endl;
//  PrintShape(out_tensor->GetShape());
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
  
  if(_same_pad) SetPaddingShift(out_tensor->GetShape());

  unsigned int out_volume = out_tensor->GetVolume();
//  std::cout<<"volume of output tensor: "<<out_volume<<std::endl;
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
    float val = _bias[i_ft];
    for(unsigned int j=0;j<ft_volume;j++){
      MIndex index0 = t_filter->GetIndexFrom1D(j);
//      std::cout<<"Get index out of 1D for filter !"<<std::endl;
      //get the corresponding element for input tensor
      MIndex index1 = AddIndex(index0,ref_index);
//      std::cout<<"Get the index for input tensor !"<<std::endl;
        
      float val0 = t_filter->GetValue(index0);
//      std::cout<<"Get value of filter "<<val0<<std::endl;
      float val1 = 0;
      if(_same_pad){
	if(GetPaddingIndex(index1,tensor->GetShape())){
          val1 = tensor->GetValue(index1);
	}
      }
      else{
        val1 = tensor->GetValue(index1);
      }

//      std::cout<<"Get Value of input tensor "<<val1<<std::endl;
      val+=val0*val1;
    }
    (*out_tensor)[out_indx] = val;
  }

  return out_tensor;
}

void MConv::SetPaddingShift(const MShape &shape){
  //add the pading information
  //here is the padding process
  //we assume the padding is symmetric and the padding value is always 0
  //p: the total padding rows or colums
  //m: size of the input tensor
  //n: size of the filter
  //s: the stride
  //0: the output size
  //the size after the padding is
  //[(m-n+2p)/s]+1 , [] means round to smaller integer
  //For the same padding, 
  //the output size should be o = [m/s]
  //the value p
  //p=(o-1)s+n-m
  //the left padding p_l = [p/2] (begin)
  //right padding p_r = p-p_l [end]
  
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

bool MConv::GetPaddingIndex(MIndex &index,const MShape& shape){
  if(index.size()==0 || shape.size()==0) return false;
  for(unsigned int i=0;i<shape.size()-1;i++){
    if(index[i]<_padding_shift0[i]) return false;
    if(index[i]>=shape[i]+_padding_shift0[i]) return false;
    index[i] = index[i]-_padding_shift0[i];
  }
  
  return true;
}

void MConv::SetBias(const float* values){
  //make sure the dimension matches
  if(!values){
    std::cout<<"MConv.cxx:: "<<WHERE<<"Null Pointer"<<std::endl;
  }
  
  for(unsigned int i=0;i<_nft;i++){
    _bias[i]=values[i];
  }
}

void MConv::SetFilter(const float* values){
  if(!values){
    std::cout<<"MConv.cxx:: "<<WHERE<<"Null Pointer"<<std::endl;
  }
  
  for(unsigned int i=0;i<_nft;i++){
    _filters[i]->Set1DValues(&(values[i*_ft_volume]));
  }
}

void MConv::SetFilter(const std::vector<float>& values){
  unsigned int total_size = _ft_volume*_nft;
  if(total_size!=values.size()){
    std::cout<<"MConv.cxx:: "<<WHERE<<" "<<"size not match!"<<std::endl;
    return;
  }
  
  std::vector<float>::const_iterator it0 = values.begin();
  std::vector<float>::const_iterator it1 = values.begin();
  for(unsigned int i=0;i<_nft;i++){
    it0=it1;
//    std::advance(it0,i*_ft_volume);
    std::advance(it1,_ft_volume);
    //create a new vector
    std::vector<float> vals(it0,it1);
    _filters[i]->Set1DValues(vals);
  }
}


void MConv::SetBias(const std::vector<float>& values){
  if(values.size()!=_nft){
    std::cout<<"MConv.cxx:: "<<WHERE<<" size not match !"<<std::endl;
  }
  
  for(unsigned int i=0;i<_nft;i++){
    _bias[i]=values[i];
  }
}

MTensor* MConv::GetFilter(unsigned int i){
  if(i<_nft) return _filters[i];
  return 0;
}


