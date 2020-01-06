#include <vector>
#include <../MIndexing.h>

void TestMFlatten(){
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
  cout<<"tensor1 shape: "<<endl;
  PrintShape(shape1);

  MFlatten* flat = new MFlatten();
  MTensor* tensor2 = flat->GetOutPut(tensor1);
  const MShape shape2 = tensor2->GetShape();
  cout<<"tensor2 shape: "<<endl;
  PrintShape(shape2);  

  MActivation* act = new MActivation(MActivation::SOFTMAX);
  MTensor* tensor3 = act->GetOutPut(tensor2);
  const MShape shape3 = tensor3->GetShape();
  PrintShape(shape3);
  
  //print the first 10 element of the tensor3
  for(unsigned int i=0;i<10;i++){
    cout<<tensor3->GetValue(i)<<endl;
  }

}
