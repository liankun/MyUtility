#include <fstream>
#include <vector>
#include "../MIndexing.h"

void CheckFirstLayer(){
  gSystem->Load("libMyUtility.so");
  ifstream in_txt("weights_first_layer_flatten_F_type.txt");
  
  vector<float> ary;
  string line;
  float values[16*2*2*8]={0.};
  float bias[16] = {0.};
  vector<float> cv_weights;
  vector<float> cv_bias;
  getline(in_txt,line);  
  stringstream ss0(line);
  for(unsigned int i=0;i<512;i++){
    ss0>>values[i];
    //if(i%2==0) values[i]=i*0.001;
    //else values[i]=i*0.001*(-1.);
    cv_weights.push_back(values[i]);
  }
  
  //bias part
  getline(in_txt,line);
  stringstream ss1(line);
  for(unsigned int i=0;i<16;i++){
    ss1>>bias[i];
    cv_bias.push_back(bias[i]);
  }
  
  MShape cov_shape = MShape(3,2);
  cov_shape[2]=8;


/*
  MConv::MConv(const MShape& shape,
                 unsigned int nft,
                 unsigned int stride,
                 bool same_pad,
		 bool set_sparse,
                 bool for_test,
                 float fill_value)
*/


  MConv* conv_2d = new MConv(cov_shape,16,1,true);
//  conv_2d->SetFilter(values);
//  conv_2d->SetBias(bias);
  conv_2d->SetFilter(cv_weights);
  conv_2d->SetBias(cv_bias);

  //check array and vector
  
  unsigned int ct=0;
  for(vector<float>::const_iterator it=cv_weights.begin();it!=cv_weights.end();it++){
    float t_val = *it;
    if(values[ct]!=t_val) cout<<"value not the same !"<<endl;
    ct++;
  }
  

/*
  for(unsigned int ct=0;ct<cv_weights.size();ct++){
    if(values[ct]!=cv_weights[ct]){
      cout<<"value not the same !"<<endl;
    }
  }
*/
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
  cout<<"Conv"<<endl;
  PrintShape(output0->GetShape());
  
  //activation
  MActivation* act = new MActivation(MActivation::RELU);
  MTensor* output1 = act->GetOutPut(output0);
  cout<<"Activation"<<endl;
  PrintShape(output1->GetShape());

  //the shape of the pool should be one less
  data_shape.pop_back();
  MShape pool_shape(2,2);
  MPooling* pool = new MPooling(pool_shape);
  MTensor* output2 = pool->GetOutPut(output1);
  cout<<"Pooling"<<endl;
  PrintShape(output2->GetShape());
  
  ifstream in_txt3("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Anaconda/test/first_layer_output.txt");
  getline(in_txt3,line);
  stringstream ss3(line);
  vector<float> fst_layer_values;
  float val;
  while(ss3>>val){
    fst_layer_values.push_back(val);
  }
  
  cout<<"Number values: "<<fst_layer_values.size()<<endl;

  MTensor* ck_tensor = output0;

  if(fst_layer_values.size()!=ck_tensor->GetVolume()){
    cout<<"size not match !!"<<endl;
    cout<<"keras size: "<<fst_layer_values.size()<<endl;
    cout<<"tensor size: "<<ck_tensor->GetVolume()<<endl;
    return;
  }
  
 
  for(unsigned int i=0;i<output0->GetVolume();i++){
    float val0 = output0->GetValue(i);
    float val1 = fst_layer_values[i];
    if(fabs(val0-val1)>1.0e-6){
      cout<<"bad news, values not the same: "<<val0<<" "<<val1<<endl;
    }
    else{
//      cout<<val0<<"  "<<val1<<endl;
    }
  }
  
}
