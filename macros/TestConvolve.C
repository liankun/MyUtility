#include<vector>
#include<fstream>
#include<iostream>

void TestConvolve(){
  gSystem->Load("libMyUtility.so");
  //Convolve a 2D tensor with a filter
  
 ifstream in_txt("test.out");
  string line;
  float mat[512][512];
  int n_lines = 0;
  while(getline(in_txt,line)){
    stringstream linestream(line);
    for(int s=0;s<512;s++){
      linestream>>mat[n_lines][s];
    }
    n_lines++;
  }

  vector<unsigned int> shape(2,512);
  
  MTensor tensor(shape,false);
  //two dim 
  vector<unsigned int> index(2,0);
  
  for(unsigned int i=0;i<512;i++){
    for(unsigned int j=0;j<512;j++){
      index[0]=i;
      index[1]=j;
      tensor[index] = mat[i][j];
    }
  }
//  tensor.Print1DTensor();
  
  float filter[2][2] = {
                        {0,1},
			{1,0}
                       };
//impelement the matrix convolve with the 2x2 filter
}
