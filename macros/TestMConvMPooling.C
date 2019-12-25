#include <vector>

void TestMConvMPooling(){
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
  
  //the last dimension is the channel
  //create a shape of (2,2,1)
  MShape cov_shape = MShape(3,2);
//  cov_shape[0]=2;
  cov_shape[2]=1;

/*
  MConv::MConv(const MShape& shape,
                 unsigned int nft,
                 unsigned int stride,
                 bool same_pad,
		 bool set_sparse,
                 bool for_test,
                 float fill_value)
*/

  MConv* cov2d = new MConv(cov_shape,1,1,true,false,true,-10000.);
  MTensor* tensor1 = cov2d->GetOutPut(tensor);
  if(!tensor1){
    cout<<"No Output for Conv !!!"<<endl;
    return;
  }

  const MShape shape1 = tensor1->GetShape();
  cout<<shape1[0]<<" "<<shape1[1]<<endl;

  //create a test Max pooling shape
  //of (2,3)
  MShape pool_shape = MShape(2,2);

  MPooling* pool = new MPooling(pool_shape);

  MTensor* tensor2 = pool->GetOutPut(tensor1);
  if(!tensor2){
    cout<<"No Output !"<<endl;
    return;
  }

  const MShape shape2 = tensor2->GetShape();
  cout<<shape2[0]<<" "<<shape2[1]<<endl;

  ifstream in_txt1("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MpcEx_CNN/Test/MConv2D/out_matrix_keras_Conv2d_Pooling.txt");
  
  const int out_size = 512;
  float out_mat[512][512];
  n_lines = 0;
  while(getline(in_txt1,line)){
    stringstream linestream(line);
    for(unsigned int s=0;s<out_size;s++){
      linestream>>out_mat[n_lines][s];
    }
    n_lines++;
  }

  //make comparision
  unsigned int volume = tensor2->GetVolume();
  for(unsigned int i=0;i<volume;i++){
    MIndex idx = tensor2->GetIndexFrom1D(i);
    float val0 = out_mat[idx[0]][idx[1]];
    float val1 = tensor2->GetValue(idx);
    if(val0!=val1){
      cout<<"bad news, value not the same !"<<endl;
      cout<<idx[0]<<"  "<<idx[1]<<endl;
      cout<<val0<<"  "<<val1<<endl;
    }
  }
}
