#include <fstream>
#include <vector>
#include "../MIndexing.h"

bool contain_alpha(string s){
//  if(s.size()==0) return false;
  for(unsigned int i=0;i<s.size();i++){
    if(isalpha(s[i]) && s[i]!='e'){
//      cout<<s[i]<<endl;
      return true;
    }
  }
  return false;
}

void readweight(){
  ifstream in_txt("weights.txt");
  
  string line;
  int n_lines=0;
  while(getline(in_txt,line)){
    if(contain_alpha(line)) cout<<line<<endl;
    //get the shape of the shape
    vector<unsigned int> shape;
    getline(in_txt,line);
    stringstream shapestream(line);
    unsigned int dim=0;
    while(shapestream>>dim) shape.push_back(dim);
    PrintShape(shape);  
    //get the number
    getline(in_txt,line);
    stringstream valuestream(line);
    vector<float> values;
    float val;
    while(valuestream>>val) values.push_back(val);
    cout<<"number of the values: "<<values.size()<<endl;

    
    cout<<"bias information !"<<endl;
    //get the shape of the bias
    shape.clear();
    getline(in_txt,line);
    stringstream ss_bias(line);
    while(ss_bias>>dim) shape.push_back(dim);
    PrintShape(shape);  
    //get the number
    getline(in_txt,line);
    stringstream ss_bias_values(line);
    values.clear();
    while(ss_bias_values>>val) values.push_back(val);
    cout<<"number of the values: "<<values.size()<<endl;

    n_lines++;
  }

  cout<<"Number lines: "<<n_lines<<endl;
}


