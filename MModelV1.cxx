#include "MModelV1.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "ExEvent.h"
#include <TMpcExShower.h>

#include "MConv.h"
#include "MPooling.h"
#include "MActivation.h"
#include "MFlatten.h"
#include "MDense.h"
#include "MIndexing.h"
#include "MPreprocess.h"

typedef std::vector<double> W;

MModelV1::MModelV1(){
  _mlayers.clear();
  _prep = new MPreprocess();
  
  //read the weights
  std::ifstream in_txt("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MyUtility/MWeights/All_weights_F_dense_C.txt");

  std::vector<W> all_weights;
  std::vector<W> all_bias;
  std::vector<MShape> all_shapes;
  std::vector<std::string> layer_names;
  std::vector<unsigned int> all_bias_size;

  std::string line;

  while(std::getline(in_txt,line)){
    layer_names.push_back(line);
    double val;
    std::vector<double> weights;
    std::vector<double> bias;
    MShape shape;
    unsigned int dim;
    
    //read shape
    std::getline(in_txt,line);
    std::stringstream ss0(line);
    while(ss0>>dim) shape.push_back(dim);
    all_shapes.push_back(shape);
    
    //read weights
    std::getline(in_txt,line);
    std::stringstream ss1(line);
    while(ss1>>val) weights.push_back(val);
    all_weights.push_back(weights);

    //read the size of the shape of bias
    std::getline(in_txt,line);
    std::stringstream ss2(line);
    while(ss2>>dim) all_bias_size.push_back(dim);

    //read bias
    std::getline(in_txt,line);
    std::stringstream ss3(line);
    while(ss3>>val) bias.push_back(val);
    all_bias.push_back(bias);
  }
  
  //build the model
  //must match the weights in the txt file
  //layer 0: (2,2,8) filter 16, stride 1, same padding
  MShape shape0(3,2);
  shape0[2]=8;
  MConv* mconv0 = new MConv(shape0,16,1,true);
  mconv0->SetWeights(all_weights[0]);
  mconv0->SetBias(all_bias[0]);
  _mlayers.push_back(mconv0);
  
  //layer 1: activation "ReLu"
  MActivation* act1 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act1);

  //layer 2:max pooling layer
  //one dimensional smaller
  MShape shape2(2,2);
  MPooling* pool2 = new MPooling(shape2,2,false,MPooling::MAX);
  _mlayers.push_back(pool2);

  //layer 3: conv (2,2,16,32)
  MShape shape3(3,2);
  shape3[2]=16;
  MConv* mconv3 = new MConv(shape3,32,1,true);
  mconv3->SetWeights(all_weights[1]);
  mconv3->SetBias(all_bias[1]);
  _mlayers.push_back(mconv3);

  //layer 4: activation "ReLu"
   MActivation* act4 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act4);

  //layer 5: max pooling layer
  MShape shape5(2,2);
  MPooling* pool5 = new MPooling(shape5,2,false,MPooling::MAX);
  _mlayers.push_back(pool5);

  //layer 6: conv (2,2,32,64)
  MShape shape6(3,2);
  shape6[2]=32;
  MConv* mconv6 = new MConv(shape6,64,1,true);
  mconv6->SetWeights(all_weights[2]);
  mconv6->SetBias(all_bias[2]);
  _mlayers.push_back(mconv6);
  
  //layer 7: activation "ReLu"
  MActivation* act7 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act7);

  //layer 8: max pooling layer
  MShape shape8(2,2);
  MPooling* pool8 = new MPooling(shape8,2,false,MPooling::MAX);
  _mlayers.push_back(pool8);

   //layer 9: (2,2,64,128)
  MShape shape9(3,2);
  shape9[2]=64;
  MConv* mconv9 = new MConv(shape9,128,1,true);
  mconv9->SetWeights(all_weights[3]);
  mconv9->SetBias(all_bias[3]);
  _mlayers.push_back(mconv9);
  
  //layer 10: activation "ReLu"
  MActivation* act10 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act10);

  //layer 11: max pooling layer
  MShape shape11(2,2);
  MPooling* pool11 = new MPooling(shape11,2,false,MPooling::MAX);
  _mlayers.push_back(pool11);

   //layer 12: conv (2,2,128,256)
  MShape shape12(3,2);
  shape12[2]=128;
  MConv* mconv12 = new MConv(shape12,256,1,true);
  mconv12->SetWeights(all_weights[4]);
  mconv12->SetBias(all_bias[4]);
  _mlayers.push_back(mconv12);
  
  //layer 13: activation "ReLu"
  MActivation* act13 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act13);

  //layer 14: max pooling layer
  MShape shape14(2,2);
  MPooling* pool14 = new MPooling(shape14,2,false,MPooling::MAX);
  _mlayers.push_back(pool14);

   //layer 15:
  MShape shape15(3,2);
  shape15[2]=256;
  MConv* mconv15 = new MConv(shape15,512,1,true);
  mconv15->SetWeights(all_weights[5]);
  mconv15->SetBias(all_bias[5]);
  _mlayers.push_back(mconv15);
  
  //layer 16: activation "ReLu"
  MActivation* act16 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act16);

  //layer 17: max pooling layer
  MShape shape17(2,2);
  MPooling* pool17 = new MPooling(shape17,2,false,MPooling::MAX);
  _mlayers.push_back(pool17);

  //layer 18: flatten
  MFlatten* flat19 = new MFlatten();
  _mlayers.push_back(flat19);

  
  //layer 19: 
  MShape shape19(2,512);
  shape19[0]=256;
  MDense* dense19 = new MDense(shape19);
  dense19->SetWeights(all_weights[6]);
  dense19->SetBias(all_bias[6]);
  _mlayers.push_back(dense19);

  //layer 19: add activation
  MActivation* act19 = new MActivation(MActivation::RELU);
  _mlayers.push_back(act19);

  
  //layer 20: ense layer
  MShape shape20(2,256);
  shape20[0]=1;
  MDense* dense20 = new MDense(shape20);
  dense20->SetWeights(all_weights[7]);
  dense20->SetBias(all_bias[7]);
  _mlayers.push_back(dense20);

  //layer 21: activation sigmoid
  MActivation* act21 = new MActivation(MActivation::SIGMOID);
  _mlayers.push_back(act21);
  
}

MModelV1::~MModelV1(){
  for(unsigned int i=0;i<_mlayers.size();i++){
    if(_mlayers[i]){
      delete _mlayers[i];
      _mlayers[i]=0;
    }
  }
  if(_prep) delete _prep;
  _prep = 0;
  _mlayers.clear();
}

std::vector<double> MModelV1::GetProb(TMpcExShower* shower){
  std::vector<double> result;
  return result;
}

std::vector<double> MModelV1::GetProb(ExShower* shower){
  std::vector<double> result;
  return result;
}

std::vector<double> MModelV1::GetProb(MTensor* tensor){
  MTensor* out_tensor=tensor;
  std::vector<double> results;
  for(unsigned int i=0;i<_mlayers.size();i++){
    MTensor* tmp_tensor = _mlayers[i]->GetOutPut(out_tensor);
    if(!tmp_tensor){
      std::cout<<"Null Output for layer: "<<i<<std::endl;
      _mlayers[i]->Print();
      return results;
    }

    //std::cout<<"Layer "<<i<<" shape "<<std::endl;
    //PrintShape(tmp_tensor->GetShape());
    if(out_tensor && i>0) delete out_tensor;
    out_tensor = tmp_tensor;
  }
  
  for(unsigned int i=0;i<out_tensor->GetVolume();i++){
    results.push_back(out_tensor->GetValue(i));
  }
  
  if(out_tensor) delete out_tensor;

  return results;
}

void MModelV1::Print(){
  //print layer info
  for(unsigned int i=0;i<_mlayers.size();i++){
    std::cout<<"Layer "<<i<<std::endl;
    _mlayers[i]->Print();
  }
}
