#include<fstream>

void CompareTwoOutput(){
  ifstream in_txt0("one_tensor_index_value_ct0.txt");
  ifstream in_txt1("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Anaconda/test/numpy_output.txt");

  int i;
  int j;
  int k;
  double val;
  
  double v0[65][65][8]={0.};
  double v1[65][65][8]={0.};

  while(in_txt0>>i>>j>>k>>val){
    v0[i][j][k]=val;
  }

  while(in_txt1>>i>>j>>k>>val){
    v1[i][j][k]=val;
  }
  
  for(int i=0;i<65;i++){
    for(int j=0;j<65;j++){
      for(int k=0;k<8;k++){
	if(v0[i][j][k]!=v1[i][j][k]){
          cout<<"value not the same !"<<endl;
	  cout<<i<<" "<<j<<" "<<k<<" "<<v0[i][j][k]<<" "<<v1[i][j][k]<<endl;
	}
      }
    }
  }
}
