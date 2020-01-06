#include <fstream>
#include <vector>
#include "../MIndexing.h"

typedef vector<float> W;

void read_all_weights(){
  vector<W> all_weights;
  vector<W> all_bias;
  vector<MShape> all_shapes;
  vector<string> layer_names;
  vector<unsigned int> all_bias_size;
  ifstream in_txt("All_weights_F.txt");
  string line;
  int nlayer=0;
  while(getline(in_txt,line)){
    
    if(line[0]=='c'){
      cout<<line<<endl;
      //convert to int
      cout<<"layer: "<<int(line[line.size()-1])-48<<endl;
    }

    layer_names.push_back(line);
    float val;
    vector<float> weights;
    vector<float> bias;
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
}
