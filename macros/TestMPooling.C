#include <vector>

void TestMPooling(){
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
  
  //the shape of the image
  //is (512,512,1)
  unsigned int nsize=512;
  MShape shape(3,nsize);
  shape[2]=1;
  
  //try sparse matrix
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
  
  //create a test Max pooling shape
  //of (2,3)
  MShape pool_shape = MShape(2,2);
//  pool_shape[1]=2;
  /*
  MPooling(const MShape& shape,
           unsigned int stride = 2,
           bool same_pad=false,
           Method md=MAX
          );
  */
 
  MPooling* pool = new MPooling(pool_shape);

  MTensor* out_tensor = pool->GetOutPut(tensor);
  if(!out_tensor){
    cout<<"No Output !"<<endl;
    return;
  }

  const MShape out_shape = out_tensor->GetShape();
  cout<<out_shape[0]<<" "<<out_shape[1]<<endl;

  ifstream in_txt1("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MpcEx_CNN/Test/MConv2D/out_matrix_keras_MaxPool_2x3_stride_2.txt");
  
  const int out_size = 255;
  float out_mat[256][255];
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
