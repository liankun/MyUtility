#include <fstream>
#include <vector>
#include "../MIndexing.h"

typedef vector<double> W;

void TestNewConv(){
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

  MTensor* data_tensor=new MTensor(data_shape,true);
  double data[33800]={0.};
  getline(in_txt1,line);
  stringstream ss4(line);
  for(unsigned int i=0;i<33800;i++){
    ss4>>data[i];  
  }
  data_tensor->Set1DValues(data);
  cout<<"Effect size of the array: "<<data_tensor->GetSparseSize()<<endl;

  PrintShape(data_tensor->GetShape());
  
  MConv* conv=0;
  for(unsigned int i=0;i<1;i++){
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
    PrintShape(t_shape);
    conv = new MConv(t_shape,t_nft,1,true);

    conv->SetFilter(all_weights[i]);
    conv->SetBias(all_bias[i]);
  }
  
  conv->Print();

  MTensor* out_tensor0 = conv->GetOutPutTest(data_tensor,true);
  PrintShape(out_tensor0->GetShape());

  MTensor* out_tensor1 = conv->GetOutPut(data_tensor,false);
  PrintShape(out_tensor1->GetShape());
  
  //check if the values are the same
  for(unsigned int i=0;i<out_tensor0->GetVolume();i++){
    double val0 = out_tensor0->GetValue(i); 
    double val1 = out_tensor1->GetValue(i);
    if(val0!=val1){
      cout<<"value not the same !"<<endl;
      cout<<i<<" "<<val0<<" "<<val1<<endl;
    }
  }
}
