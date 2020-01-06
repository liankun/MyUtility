#include <fstream>
#include <vector>
#include "../MIndexing.h"
#include <string>

void TestDense(){
  ifstream in_txt("ex_example_flatten_out.txt");
  vector<float> ex_values;
  float val;
  while(in_txt>>val) ex_values.push_back(val);
  cout<<"number of the example values: "<<ex_values.size()<<endl;

  gSystem->Load("libMyUtility.so");

  MShape data_shape(1,ex_values.size());
  MTensor* data_tensor = new MTensor(data_shape);
  data_tensor->Set1DValues(ex_values);
  
//  for(unsigned int i=0;i<data_tensor->GetVolume();i++){
//    cout<<i<<" "<<data_tensor->GetValue(i)<<endl;
//  }

  MShape dense_shape(2,256);
  dense_shape[1]=512;

  MDense* dense = new MDense(dense_shape);
  vector<float> weights;
  vector<float> bias;
  /*
  for(unsigned int i=0;i<256;i++){
    for(unsigned int j=0;j<512;j++){
      weights.push_back(1); 
    }
    bias.push_back(1);
  }
  */
  string line;
  ifstream in_txt1("dense_weight.txt");
  getline(in_txt1,line);
  stringstream ss0(line);
  while(ss0>>val) weights.push_back(val);
  cout<<"Number of weights: "<<weights.size()<<endl;

  getline(in_txt1,line);
  stringstream ss1(line);
  while(ss1>>val) bias.push_back(val);
  cout<<"Number of bias: "<<bias.size()<<endl;

  dense->SetWeights(weights);
  dense->SetBias(bias);

  MTensor* out_tensor = dense->GetOutPut(data_tensor);
  cout<<"OutPut Shape:"<<endl;
  PrintShape(out_tensor->GetShape());

//  for(unsigned int i=0;i<out_tensor->GetVolume();i++){
//    cout<<out_tensor->GetValue(i)<<endl;
//  }

  ifstream in_txt3("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Anaconda/test/first_layer_output.txt");
  getline(in_txt3,line);
  stringstream ss5(line);
  vector<float> fst_layer_values;
  while(ss5>>val){
    fst_layer_values.push_back(val);
  }
  
  if(fst_layer_values.size()!=out_tensor->GetVolume()){
    cout<<"size not match !"<<endl;
    return;
  }

  for(unsigned int i=0;i<out_tensor->GetVolume();i++){
    float val0 = fst_layer_values[i];
    float val1 = out_tensor->GetValue(i);
    
    if(fabs(val0-val1)>1.0e-6){
      cout<<"bad news, values not the same !"<<endl;
      cout<<"keras: "<<val0<<" tensor: "<<val1<<endl;
    }

  }
}
