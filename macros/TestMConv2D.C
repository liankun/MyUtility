#include <vector>

void TestMConv2D(){
  gSystem->Load("libMyUtility.so");

  ifstream in_txt("test_matrix.txt");
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
  
  unsigned int nsize=512;
  MShape shape(3,nsize);
  shape[2]=1;
 
  MTensor* tensor = new MTensor(shape);
  //two dim 
  MIndex index(3,0);
  index[2]=0;
  
  for(unsigned int i=0;i<nsize;i++){
    for(unsigned int j=0;j<nsize;j++){
      index[0]=i;
      index[1]=j;
      (*tensor)[index] = mat[i][j];
    }
  }
  
  unsigned int stride = 1;

  MConv2D* cov2d = new MConv2D(stride,1.);
  MTensor* out_tensor = cov2d->GetOutPut(tensor);
  if(!out_tensor) return;
//  if(out_tensor) out_tensor->Print1DTensor();

  const MShape out_shape = out_tensor->GetShape();
  cout<<out_shape[0]<<" "<<out_shape[1]<<endl;

  
  //calculate by another method

  float out_mat[out_shape[0]][out_shape[1]]={0};

  float ft[2][2]={
                  {1,1},
		  {1,1}
                 };
  for(unsigned int i=0;i<nsize-2+1;i+=stride){
    for(unsigned int j=0;j<nsize-2+1;j+=stride){
      float val=0;
      unsigned int row = i/stride;
      unsigned int col = j/stride;
  //    cout<<"row: "<<row<<" col: "<<col<<endl;
      for(unsigned int m=0;m<2;m++){
	for(unsigned int n=0;n<2;n++){
//	  cout<<mat[i+m][j+n]<<"  "<<ft[m][n]<<endl;
          val+=mat[i+m][j+n]*ft[m][n];
	}
      }
      out_mat[row][col] = val;
    }
  }

  //make comparision
  unsigned int volume = out_tensor->GetVolume();
  for(unsigned int i=0;i<volume;i++){
    MIndex idx = out_tensor->GetIndexFrom1D(i);
    float val0 = out_mat[idx[0]][idx[1]];
    float val1 = out_tensor->GetValue(idx);
    if(val0!=val1){
      cout<<"bad news, value not the same !"<<endl;
      cout<<idx[0]<<"  "<<idx[1]<<endl;
      cout<<val0<<"  "<<val1<<endl;
    }
  }
  cout<<"finish test"<<endl;
}
