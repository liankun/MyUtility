#include "../MIndexing.h"
#include <fstream>
#include <vector>

typedef vector<double> V;

void TestMModelV1(){
  gSystem->Load("libMyUtility.so");

  MModelV1* model_v1 = new MModelV1();
  
  ifstream in_txt("tensor_out_example_68examples.txt");
  string line;
  vector<V> all_examples;
  while(getline(in_txt,line)){
    V single_example;
    stringstream ss(line);
    double val;
    while(ss>>val) single_example.push_back(val);
    all_examples.push_back(single_example);
  }

  cout<<"Number of example: "<<all_examples.size()<<endl;
  
  MShape shape(3,65);
  shape[2]=8;
  MTensor* mt = new MTensor(shape);
  ofstream out_txt("final_output_modelv1.txt");
  for(unsigned int i=0;i<all_examples.size();i++){
    mt->Set1DValues(all_examples[i]); 
    vector<double> prob = model_v1->GetProb(mt);
    for(unsigned int j=0;j<prob.size();j++){
      out_txt<<prob[j]<<" ";
    }
    out_txt<<endl;
  }
}
