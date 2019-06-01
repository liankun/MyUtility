#include "MiniClusterV1.h"

MiniClusterV1::MiniClusterV1(){
  Reset();
}

MiniClusterV1::~MiniClusterV1(){
  Reset();
}

void MiniClusterV1::Reset(){
  _is_seed_pk = false;
  _rms_x = -9999;
  _rms_y = -9999;
  _radius = -9999;
  memset(_layer_pk_e,0,sizeof(_layer_pk_e));
  memset(_layer_ex_e,0,sizeof(_layer_ex_e));

  for(unsigned int i=0;i<_sq_list.size();i++){
    if(_sq_list[i]){
      delete _sq_list[i];
      _sq_list[i] = NULL;
    }
  }
  _sq_list.clear();
}

double MiniClusterV1::GetPkE(){
  double sum=0;
  for(int i=0;i<8;i++){
    sum+=_layer_pk_e[i];
  }
  return sum;

}

double MiniClusterV1::GetExE(){
  double sum=0;
  for(int i=0;i<8;i++){
    sum+=_layer_ex_e[i];
  }
  return sum;
}


