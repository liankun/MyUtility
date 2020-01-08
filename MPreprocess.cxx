#include "MPreprocess.h"
#include "ExEvent.h"
#include <TMpcExShower.h>
#include <TMpcExHitContainer.h>
#include <TMpcExHit.h>
#include "MTensor.h"
#include <iostream>
#include <MpcExMapper.h>

MPreprocess::MPreprocess(const TMpcExHitContainer* hits){
  MPreprocess();
  SetHitContainer(hits);
}

MPreprocess::MPreprocess(){
  //default a shape of (65,65,8)
  _shape.push_back(NSPACE);
  _shape.push_back(NSPACE);
  _shape.push_back(8);
  _ex_hit_map.clear();
}

MPreprocess::~MPreprocess(){
}

MTensor* MPreprocess::GetTensor(TMpcExShower* shower,bool set_sparse){
  MTensor* t = new MTensor(_shape,set_sparse);
  if(SetTensor(t,shower)) return t;
  else{
    delete t;
    return 0;
  }
}

MTensor* MPreprocess::GetTensor(ExShower* shower,bool set_sparse){
  MTensor* t = new MTensor(_shape,set_sparse);
  if(SetTensor(t,shower)) return t;
  else{
    delete t;
    return 0;
  }
}

bool MPreprocess::SetTensor(MTensor* t,TMpcExShower* shower){
  if(!IndexEqual(_shape,t->GetShape())){
    std::cout<<"MPreprocess.cxx:: Shape not the same!"<<std::endl;
    return false;
  }

  if(_ex_hit_map.size()==0){
    std::cout<<"MPreprocess.cxx:: Set TMpcExContainer first !"<<std::endl;
    return false;
  }

  t->Clear();

  unsigned int nhits = shower->sizeHits();
  //get the shower center first
  float vertex = shower->get_vertex();
  float z = 205;
  if(shower->get_arm()==0) z = -205;
  float ct_x = shower->get_hsx()*(z-vertex);
  float ct_y = shower->get_hsy()*(z-vertex);
  //GeV->KeV
  float raw_ex_e = shower->get_raw_esum()*1.0e6;

  for(unsigned int i=0;i<nhits;i++){
    int key = shower->getHit(i);
    TMpcExHit* hit = _ex_hit_map[key];
    float x = hit->x()-ct_x;
    float y = hit->y()-ct_y;
    float e = hit->combined()/raw_ex_e;
    int layer = hit->layer();

    bool is_valid = SetTensorByHit(layer,x,y,e,t);
    if(!is_valid) return false;
  }

  return true;
}

bool MPreprocess::SetTensor(MTensor* t,ExShower* shower){
  if(!IndexEqual(_shape,t->GetShape())){
    std::cout<<"MPreprocess.cxx:: Shape not the same!"<<std::endl;
    return false;
  }

  t->Clear();

  MpcExMapper* ex_map = MpcExMapper::instance();
  int nhits = shower->GetNhits();
  float vertex = shower->GetVertex();
  float z=205;
  if(shower->GetArm()==0) z=-205;
  float ct_x = shower->GetHsx()*(z-vertex);
  float ct_y = shower->GetHsy()*(z-vertex);
  
  //GeV->keV
  float raw_ex_e = shower->GetRawMpcexE()*1.0e6;
  MIndex index(3,0);

  for(int i=0;i<nhits;i++){
    ExHit* hit = shower->GetExHit(i);
    int key = hit->GetKey();
    float x = ex_map->get_x(key)-ct_x;
    float y = ex_map->get_y(key)-ct_y;
    float e = hit->GetE()/raw_ex_e;
    int layer = ex_map->get_layer(key);

    bool is_valid=SetTensorByHit(layer,x,y,e,t);
    if(!is_valid) return false;
  }
  return true;
}

bool MPreprocess::SetTensorByHit(int layer,float x,float y,float e,MTensor* t){
  if(!t){
    std::cout<<"MPreprocess:: Null Pointer !"<<std::endl;
    return false;
  }
  if(!IndexEqual(_shape,t->GetShape())){
    std::cout<<"MPreprocess.cxx:: Shape not the same!"<<std::endl;
    return false;
  }

  MIndex index(3,0);

  int binx0 = int((x+SHOWERSIZE)/DSPACE);
  int binx1 = binx0+1;
  int biny0 = int((y+SHOWERSIZE-DSPACE/2.*7.)/DSPACE);
  int biny1 = int((y+SHOWERSIZE+DSPACE/2.*7.)/DSPACE)+1;

  if(layer%2==1){
    biny0 = int((y+SHOWERSIZE)/DSPACE);
    biny1 = biny0+1;
    binx0 = int((x+SHOWERSIZE-DSPACE/2.*7)/DSPACE);
    binx1 = int((x+SHOWERSIZE+DSPACE/2.*7)/DSPACE)+1;
  }
     
  for(int ix=binx0;ix<binx1;ix++){
    for(int iy=biny0;iy<biny1;iy++){
      /*
      if((NSPACE-ix)<0||(NSPACE-ix)>=NSPACE){
          continue;
      }
      if(iy<0||iy>=NSPACE){
	continue;
      }
      index[0]=NSPACE-ix;
      index[1]=iy;
      index[2]=layer;
      (*t)[index]=e;
      */
      if(ix<0||ix>=NSPACE){
          continue;
      }
      if(iy<0||iy>=NSPACE){
	continue;
      }
      index[0]=ix;
      index[1]=iy;
      index[2]=layer;
      (*t)[index]=e;

    }
  }
  return true;
}

void MPreprocess::SetShape(MShape& shape){
  _shape = shape;
}

void MPreprocess::SetHitContainer(const TMpcExHitContainer* hits){
  if(!hits){
    std::cout<<"Null Hits container !"<<std::endl;
  }
  else{
    for(unsigned int i=0;i<hits->size();i++){
      TMpcExHit* hit = hits->getHit(i);
      int key = hit->key();
      _ex_hit_map[key]=hit;
    }
  }
}
