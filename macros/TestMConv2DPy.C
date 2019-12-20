#include <vector>

void TestMConv2DPy(){
  gSystem->Load("libMyUtility.so");

  ifstream in_txt("test_matrix.txt");
  string line;
  const int in_size = 512;
  float mat[in_size][in_size];
  int n_lines = 0;
  while(getline(in_txt,line)){
    stringstream linestream(line);
    for(unsigned int s=0;s<in_size;s++){
      linestream>>mat[n_lines][s];
    }
    n_lines++;
  }
  
  unsigned int nsize=512;
  MShape shape(3,nsize);
  shape[2]=1;
  
  //try sparse matrix
  MTensor* tensor = new MTensor(shape,true);
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
  
  //the last dimension is the channel
  //create a shape of (2,3,1)
  MShape cov_shape = MShape(3,3);
  cov_shape[0]=2;
  cov_shape[2]=1;

/*
  MConv2D::MConv2D(const MShape& shape,
                 unsigned int nft,
                 unsigned int stride,
                 bool same_pad,
		 bool set_sparse,
                 bool for_test,
                 float fill_value)
*/
  MConv2D* cov2d = new MConv2D(cov_shape,1,1,false,true,true,1);
  MTensor* out_tensor = cov2d->GetOutPut(tensor);
  if(!out_tensor) return;
//  if(out_tensor) out_tensor->Print1DTensor();

  const MShape out_shape = out_tensor->GetShape();
  cout<<out_shape[0]<<" "<<out_shape[1]<<endl;

  //calculate by another method

  ifstream in_txt1("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MpcEx_CNN/Test/MConv2D/out_maxtrix.txt");
  
  const int out_size = 510;
  float out_mat[511][out_size];
  n_lines = 0;
  while(getline(in_txt1,line)){
    stringstream linestream(line);
    for(unsigned int s=0;s<out_size;s++){
      linestream>>out_mat[n_lines][s];
    }
    n_lines++;
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
}
