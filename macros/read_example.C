#include <fstream>
#include <vector>

void read_example(){
  gSystem->Load("libMyUtility.so");
  ifstream in_txt("test_ex_example_flatten_F_type.txt");
  vector<float> ary;
  string line;
  float values[33800]={0.};
  while(getline(in_txt,line)){
    float val;
    stringstream ss(line);
    while(ss>>val) ary.push_back(val);
  }
  for(unsigned int i=0;i<ary.size();i++){
    values[i]=ary[i];
  }
  cout<<"Number of size: "<<ary.size()<<endl;

  MShape shape(3,65);
  shape[2]=8;
  
  MTensor* tensor = new MTensor(shape);
  tensor->Set1DValues(values);

  MIndex index(3,0);
  float value;
  ifstream in_txt2("test_ex_example_index.txt");
  while(in_txt2>>index[0]>>index[1]>>index[2]>>value){
    float t_value = tensor->GetValue(index);
    if(value!=t_value){
      cout<<index[0]<<" "<<index[1]<<" "<<index[2]<<endl;
      cout<<value<<" "<<t_value<<endl;
      cout<<"value different !"<<endl;
    }
  }
}
