#include "MiniClusterV1.h"
#include <iostream>

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
  _pk_x = -9999;
  _pk_y = -9999;
  _x = -9999;
  _y = -9999;
  _pk_ix = -9999;
  _pk_iy = -9999;
  _radius = -9999;

  _sq_nxn_e.clear();
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

void MiniClusterV1::Print(){
  std::cout<<"ExE: "<<GetExE()
           <<" PkE: "<<GetPkE()
           <<" x: "<<GetX()
	   <<" y: "<<GetY()
	   <<" RMS X: "<<GetRMSX()
	   <<" RMS Y: "<<GetRMSY()
	   <<" RMS R: "<<GetRMS()
	   <<" NSquare: "<<GetNSq()
	   <<std::endl;

}

MiniCluster* MiniClusterV1::Clone(){
  MiniClusterV1* mclus = new MiniClusterV1();
  mclus->SetRMSX(_rms_x);
  mclus->SetRMSY(_rms_y);
  mclus->SetPkX(_pk_x);
  mclus->SetPkY(_pk_y);
  mclus->SetPkGridX(_pk_ix);
  mclus->SetPkGridY(_pk_iy);
  mclus->SetX(_x);
  mclus->SetY(_y);
  mclus->SetRadius(_radius);
  mclus->SetIsSeedPk(_is_seed_pk);
  mclus->SetShowerE(_shower_e);
  mclus->SetVertex(_vertex);
  for(int i=0;i<8;i++){
    mclus->SetLayerE(i,_layer_ex_e[i]);
    mclus->SetLayerPkE(i,_layer_pk_e[i]);
  }
  
  for(unsigned int i=0;i<_sq_nxn_e.size();i++){
    mclus->InsertSqENxN(_sq_nxn_e[i]);
  }

  for(unsigned int i=0;i<_sq_list.size();i++){
    mclus->InsertSq(_sq_list[i]->Clone());
  }
  
  return (MiniCluster*)mclus;
}
