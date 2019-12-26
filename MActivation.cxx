#include "MActivation.h"
#include "MTensor.h"
#include <iostream>
#include <math.h>

MActivation::MActivation(Function fc){
  _fc = fc;
}

MTensor* MActivation::GetOutPut(MTensor* tensor,bool set_sparse){
  MTensor* out_tensor = new MTensor(tensor->GetShape(),set_sparse);
  
  //denumerator factor for softmax, need sum along the last 
  //layer
  //for softmax activation, which 
  //need the sum up the channel 
  //for the shape size >=2
  MTensor* de_tensor=0;
  //for the single value 
  float de_value=0;
  if(tensor->GetNDim()>=2){
    if(_fc>=SOFTMAX){
      std::cout<<"SOFTMAX NDIM>=2"<<std::endl;
      MShape tmp_shape = tensor->GetShape();
      //reduce the dimension by 1
      unsigned int last_idx = tmp_shape.size()-1;
      //get the number of the channels (last dimension)
      unsigned int nchannel = tmp_shape[last_idx];
      std::cout<<"N Last Channel: "<<nchannel<<std::endl;
      tmp_shape.pop_back();
      std::cout<<"New Created tensor shape:"<<std::endl;
      PrintShape(tmp_shape);
      de_tensor = new MTensor(tmp_shape);
      for(unsigned int j=0;j<de_tensor->GetVolume();j++){
        MIndex tmp_index = de_tensor->GetIndexFrom1D(j);
        tmp_index.push_back(0);
        float sum_val=0;
        for(unsigned int k=0;k<nchannel;k++){
	  //only the last index the same
          tmp_index[last_idx]=k; 
	  sum_val+=exp(tensor->GetValue(tmp_index));
        }
        (*de_tensor)[j]=sum_val;
      }
    }
  }

  if(tensor->GetNDim()==1){
    if(_fc>=SOFTMAX){
      std::cout<<"SOFTMAX NDim == 1"<<std::endl;
      for(unsigned int k=0;k<tensor->GetVolume();k++){
        de_value+=tensor->GetValue(k);
      }
    }
  }
  
  for(unsigned int i=0;i<out_tensor->GetVolume();i++){
    float val = tensor->GetValue(i);
    float out_val = GetActValue(val);
    if(_fc>=SOFTMAX){
      if(de_tensor){
        MIndex de_index = tensor->GetIndexFrom1D(i);
	//only interested in the index except the last one
	de_index.pop_back();
	out_val=out_val/de_tensor->GetValue(de_index);
      }
      else{
        if(de_value!=0) out_val=out_val/de_value;
	else std::cout<<"MActivation:: "<<WHERE<<" bad de_value !"<<std::endl;
      }
    }
    
    (*out_tensor)[i]=out_val;    
  }
  
  if(de_tensor) delete de_tensor;

  return out_tensor;
}

void MActivation::Print(){
  std::cout<<"Layer Activation!"<<std::endl;
  switch(_fc){
    case RELU:
      std::cout<<"Type RELU"<<std::endl;
      break;
    case SOFTMAX:
      std::cout<<"Type SOFTMAX"<<std::endl;
      break;
    case SIGMOID:
      std::cout<<"Type SIGMOID"<<std::endl;
      break;
    default:
      std::cout<<"Unknow !"<<std::endl;
  }
}

float MActivation::GetActValue(float val){
  switch(_fc){
    case RELU:
      if(val>0) return val;
      return 0;
    case SIGMOID:
      return 1./(1+exp(-val));
    case SOFTMAX:
      return exp(val);
    default:
      std::cout<<"Unknow function !"<<std::endl;
      return 0;
  } 
}
