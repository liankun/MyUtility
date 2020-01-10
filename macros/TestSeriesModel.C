#include <fstream>
#include <vector>
#include "../MIndexing.h"

typedef vector<double> W;


void TestSeriesModel(){
  vector<W> all_weights;
  vector<W> all_bias;
  vector<MShape> all_shapes;
  vector<string> layer_names;
  vector<unsigned int> all_bias_size;
  ifstream in_txt("All_weights_F_dense_C.txt");
  string line;
  int nlayer=0;
  while(getline(in_txt,line)){
    
    if(line[0]=='c'){
      cout<<line<<endl;
      //convert to int
      cout<<"layer: "<<int(line[line.size()-1])-48<<endl;
    }

    layer_names.push_back(line);
    double val;
    vector<double> weights;
    vector<double> bias;
    MShape shape;
    unsigned int dim;
    
    //read shape
    getline(in_txt,line);
    stringstream ss0(line);
    while(ss0>>dim) shape.push_back(dim);
    all_shapes.push_back(shape);
    
    //read weights
    getline(in_txt,line);
    stringstream ss1(line);
    while(ss1>>val) weights.push_back(val);
    all_weights.push_back(weights);

    //read the size of the shape of bias
    getline(in_txt,line);
    stringstream ss2(line);
    while(ss2>>dim) all_bias_size.push_back(dim);

    //read bias
    getline(in_txt,line);
    stringstream ss3(line);
    while(ss3>>val) bias.push_back(val);
    all_bias.push_back(bias);
  }
  
  for(unsigned int i=0;i<all_shapes.size();i++){
    cout<<"layer "<<i<<endl;
    cout<<layer_names[i]<<endl;
    cout<<"Shape: ";
    PrintShape(all_shapes[i]);
    cout<<"size of weights: "<<all_weights[i].size()<<endl;
    cout<<"size of bias: "<<all_bias_size[i]<<endl;
    cout<<"size of bias: "<<all_bias[i].size()<<endl;
  }
  

  gSystem->Load("libMyUtility.so");
  //read example data
  ifstream in_txt1("test_ex_example_flatten_F_type.txt"); 
  
  MShape data_shape(3,65);
  data_shape[2]=8;

  MTensor* data_tensor=new MTensor(data_shape);
  double data[33800]={0.};
  getline(in_txt1,line);
  stringstream ss4(line);
  for(unsigned int i=0;i<33800;i++){
    ss4>>data[i];  
  }
  data_tensor->Set1DValues(data);

  //start build model
  
  vector<MLayer*> mlayers_list;

  for(unsigned int i=0;i<6;i++){
    MShape t_shape = all_shapes[i];
    unsigned int t_nft=all_shapes[i][t_shape.size()-1];
    unsigned int nsize=1;
    //get the total size
    for(unsigned int j=0;j<t_shape.size();j++){
      nsize*=t_shape[j];
    }
    //last one is the number of the filter
    t_shape.pop_back();
    //shape, number of filter,stride=1,same padding
//    cout<<"conv"<<endl;
    PrintShape(t_shape);
//    cout<<"Number of filter: "<<t_nft<<endl;
//    cout<<"Number of total weights: "<<all_weights[i].size()<<endl;
    MLayer* conv = new MConv(t_shape,t_nft,1,true);

//    conv->SetFilter(all_weights[i].begin(),all_weights[i].end());
//    conv->SetBias(all_bias[i].begin(),all_bias[i].end());
    conv->SetFilter(all_weights[i]);
    conv->SetBias(all_bias[i]);
    mlayers_list.push_back(conv);
    //activation
//    cout<<"activation"<<endl;
    MActivation* act = new MActivation(MActivation::RELU);
    mlayers_list.push_back(act);
    //the pooling layer 
    t_shape.pop_back();
//    cout<<"pooling"<<endl;
    MPooling* pool = new MPooling(t_shape,2,false,MPooling::MAX);
    pool->Print();
    mlayers_list.push_back(pool); 
  }

  //add flatten layer
  MFlatten* flat = new MFlatten();
  mlayers_list.push_back(flat); 

  //dense layer
  MShape d_shape0(2,512);
  d_shape0[0]=256;
  MDense* dense0 = new MDense(d_shape0);
  dense0->SetWeights(all_weights[6]);
  dense0->SetBias(all_bias[6]);
  mlayers_list.push_back(dense0);

  //add activation
  MActivation* act = new MActivation(MActivation::RELU);
  mlayers_list.push_back(act);
  
  //add last dense layer
  MShape d_shape1(2,256);
  d_shape1[0]=1;
  MDense* dense1 = new MDense(d_shape1);
  dense1->SetWeights(all_weights[7]);
  dense1->SetBias(all_bias[7]);
  mlayers_list.push_back(dense1);
  
  //add last activation
  act = new MActivation(MActivation::SIGMOID);
  mlayers_list.push_back(act);

  cout<<"print layer information: "<<endl;
  for(unsigned int i=0;i<mlayers_list.size();i++){
    mlayers_list[i]->Print();
  }
  cout<<endl;

  cout<<"Calculate the output !"<<endl;
  MTensor* in_tensor=data_tensor;
  
  cout<<"numer of the layers "<<mlayers_list.size()<<endl;
  

  for(unsigned int i=0;i<mlayers_list.size();i++){
    MTensor* output = mlayers_list[i]->GetOutPut(in_tensor);
    mlayers_list[i]->Print();
    cout<<"Layer "<<i;
    cout<<" OutPut Shape: "<<endl;
    PrintShape(output->GetShape());
//    delete in_tensor;
    in_tensor = output;
  }
 
  if(!in_tensor){
    cout<<"invalid output tensor !"<<endl;
    return;
  }

  ifstream in_txt3("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Anaconda/test/first_layer_output.txt");
  getline(in_txt3,line);
  stringstream ss5(line);
  vector<double> fst_layer_values;
  double val;
  while(ss5>>val){
    fst_layer_values.push_back(val);
  }
  
  if(fst_layer_values.size()!=in_tensor->GetVolume()){
    cout<<"size not match !"<<endl;
    cout<<"input size: "<<fst_layer_values.size()<<endl;
    cout<<"tensor size: "<<in_tensor->GetVolume()<<endl;
    return;
  }
  
  cout<<"Number values: "<<fst_layer_values.size()<<endl;

  for(unsigned int i=0;i<in_tensor->GetVolume();i++){
    double val0 = in_tensor->GetValue(i);
    double val1 = fst_layer_values[i];
    if(fabs(val0-val1)>1.0e-6){
      cout<<"bad news, values not the same: "<<val0<<" "<<val1<<endl;
    }
    else{
      cout<<val0<<"  "<<val1<<endl;
    }
  }

//  ofstream out_txt1("ex_example_flatten_out.txt");
//  for(unsigned int i=0;i<in_tensor->GetVolume();i++){
//    out_txt1<<in_tensor->GetValue(i)<<endl;
//  } 
}
