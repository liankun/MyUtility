#include<vector>


void get_size(unsigned int (&a)[5]){
  cout<<"size of the array: "<<sizeof(a)<<endl;
}

void print_info(){
  cout<<"print information !"<<endl;
}

void TestMTensor(){
  unsigned int a[5] = {0,0,0,0,0};
  cout<<"size of the array: "<<sizeof(a)<<endl;
//  print_info();
//  get_size(a);

  gSystem->Load("libMyUtility.so");
//  vector<int> v{ 1,2,3 };
//  cout<<v.size()<<endl;
  vector<unsigned int> v(4,0);
  v[0]=1;
  v[1]=2;
  v[2]=3;
  v[3]=4;
  cout<<v.size()<<endl;
  int myints[] = {16,2,77,29};
  cout<<sizeof(myints)<<endl;
  
  MTensor tensor(v,false);
  vector<unsigned int> idx(5,0);
  vector<unsigned int> idx1(4,0);
  idx1[0]=0;
  idx1[1]=0;
  idx1[2]=2;
  idx1[3]=3;

  tensor[idx1]=1;

  tensor.Print1DTensor();
}


