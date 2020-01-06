#include <vector>
#include "../MIndexing.h"

void TestReadWeight(){
  gSystem->Load("libMyUtility.so");
  ifstream in_txt("weights_first_layer_flatten_C_type.txt");
  
  vector<float> ary;
  string line;
  float values[16*2*2*8]={0.};
  float bias[16] = {0.};
  getline(in_txt,line);  
  stringstream ss0(line);
  for(unsigned int i=0;i<512;i++){
    ss0>>values[i];
  }
  
  //bias part
  getline(in_txt,line);
  stringstream ss1(line);
  for(unsigned int i=0;i<16;i++){
    ss1>>bias[i];
  }
  
  
  MShape cov_shape = MShape(3,2);
  cov_shape[2]=8;

  MConv* conv_2d = new MConv(cov_shape,16,1,true);
  conv_2d->SetFilter(values);
  conv_2d->SetBias(bias);

  ifstream in_txt2("test_ex_example_flatten_F_type.txt"); 
  
  MShape data_shape(3,65);
  data_shape[2]=8;

  MTensor* data_tensor=new MTensor(data_shape);

  float data[33800]={0.};
  getline(in_txt2,line);
  stringstream ss2(line);

  for(unsigned int i=0;i<33800;i++){
    ss2>>data[i];  
  }
  
  data_tensor->Set1DValues(data);

  MTensor* output0 = conv_2d->GetOutPut(data_tensor);
  PrintShape(output0->GetShape());
}
