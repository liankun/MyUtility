#include "STFinderV2.h"
#include <iostream>
#include <MpcExMapper.h>
#include <MpcExConstants.h>
#include <set>

using namespace std;

void STFinderV2::SetExShower(ExShower* ex_shower){
  _hit_map.clear();
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();  
  int nhits = ex_shower->GetNhits();
  for(int i=0;i<nhits;i++){
    ExHit* hit = ex_shower->GetExHit(i);
    int key = hit->GetKey();
    int layer = mpcex_mapper->get_layer(key);
    int arm = mpcex_mapper->get_arm(key);
    int nx = mpcex_mapper->get_nx(key);
    int ny = mpcex_mapper->get_ny(key);
    
    //the north arm ny seems reversed
    if(arm == 1){
      if(layer%2 == 0) ny = 23 - ny;
      else ny = 197 - ny;
    }
    
    int index = nx+198*ny+198*6*4*layer+198*4*6*8*arm;
    if(layer%2 == 1){
      index = ny+198*nx+198*6*4*layer+198*4*6*8*arm;
    }
    _hit_map.insert(pair<int,ExHit*>(index,hit));
  }
  _vertex = ex_shower->GetVertex();
  _nfired = ex_shower->GetFiredLayers();
  _shower_e = ex_shower->GetTotalE();
}


//for ny used the ny get by STFinderV2 (not from MpcExMapper)
int STFinderV2::get_index(int arm,int layer,int nx,int ny) const{
  unsigned index = 0;
  if(arm > 1 || layer > 7){
    std::cout <<"bad arm or layer value "<<std::endl;
    return 0;
  }
    
  if(layer%2 == 0){
    if(nx > 197 || ny > 23){
      return 0;
    }
    index = nx+198*ny+198*6*4*layer+198*4*6*8*arm;
  }
  if(layer%2 == 1){
    if(nx > 23 || ny > 197){
      return 0;
    }
    index = ny+198*nx+198*6*4*layer+198*4*6*8*arm;
  }
  return index;
}

int STFinderV2::get_index(ExHit* hit) const{
   if(!hit){
    std::cout<<"bad hit"<<std::endl;
    return 0;
  }
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  int key = hit->GetKey();
  int arm = mpcex_mapper->get_arm(key);
  int layer = mpcex_mapper->get_layer(key);
  int nx = get_nx(hit);
  int ny = get_ny(hit);
  
  return get_index(arm,layer,nx,ny);
}

int STFinderV2::get_nx(ExHit* hit) const {
  if(!hit){
    std::cout<<"bad hit"<<std::endl;
    return 0;
  }
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  int key = hit->GetKey();
  int nx = mpcex_mapper->get_nx(key);
  return nx;
}

int STFinderV2::get_ny(ExHit* hit) const {
  if(!hit) {
    std::cout<<"bad hit"<<std::endl;
    return 0;
  }
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  int key = hit->GetKey();
  unsigned int arm = mpcex_mapper->get_arm(key);
  unsigned int layer = mpcex_mapper->get_layer(key);
  unsigned int ny = mpcex_mapper->get_ny(key);;
  if(arm == 1){
    if(layer%2 == 0) ny = 23 - ny;
    else ny = 197 - ny;
  }
  return ny;
}

void STFinderV2::Search(){
   for(unsigned int i=0;i<_track_lists.size();i++){
     if(_track_lists[i]) delete _track_lists[i];
   }
   _track_lists.clear();


   map<int,ExHit*>::iterator it = _hit_map.begin();
   vector<ExHit*> pk_minipads[8]; 
   
   //four direction
   int suround[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
   MpcExMapper* mpcex_mapper = MpcExMapper::instance();
   //search the peak minipad in each layer.
   //if the e large than the 4 neighbor
   for(;it!=_hit_map.end();it++){
     ExHit* hit = it->second;
     int key = hit->GetKey();
     int arm = mpcex_mapper->get_arm(key);
     int layer = mpcex_mapper->get_layer(key);
     //using own method of nx and ny
     //In MpcExMapper, ny in north arm may be reversed
     int nx = get_nx(hit); 
     int ny = get_ny(hit);
     bool is_peak = true;
     double q = hit->GetE();
     for(int idir=0;idir<4;idir++){
       int neighbor_index = get_index(arm,layer,nx+suround[idir][0],ny+suround[idir][1]);
       if(_hit_map.find(neighbor_index) == _hit_map.end()) continue;
       ExHit* neighbor_hit = _hit_map[neighbor_index];
       double neighbor_q = neighbor_hit->GetE();
       if(neighbor_q > q) is_peak = false;
     }
     if(is_peak) pk_minipads[layer].push_back(hit);
   }

   //use the vertex to make alignment the peaks
   double pad_short = MpcExConstants::MINIPAD_SHORT_LENGTH*1.5;
   double pad_long = MpcExConstants::MINIPAD_LONG_LENGTH*0.6;
   
   //x type list
   set<int> used_list_x;
   //y type list
   set<int> used_list_y;
   for(int ilayer=0;ilayer<8;ilayer++){
     if(pk_minipads[ilayer].size() <=0) continue;
     int npk = pk_minipads[ilayer].size();
     for(int ipk=0;ipk<npk;ipk++){
       ExHit* pk_pad = pk_minipads[ilayer][ipk];
       int pk_key = pk_pad->GetKey();
       double pk_x = mpcex_mapper->get_x(pk_key);
       double pk_y = mpcex_mapper->get_y(pk_key);
       double pk_z = mpcex_mapper->get_z(pk_key);
       double pk_hsx = pk_x/(pk_z-_vertex);
       double pk_hsy = pk_y/(pk_z-_vertex);
       int index = get_index(pk_pad); 
       if(ilayer%2==0){
         if(used_list_x.find(index)!=used_list_x.end()) continue;
         used_list_x.insert(index);  
       }
       else{
         if(used_list_y.find(index)!=used_list_y.end()) continue;
         used_list_y.insert(index);  
       }
       Track* track = new Track();
       track->push_back(pk_pad);
       //loop in the same layer to find the matched peaks
       for(int jlayer=ilayer+1;jlayer<8;jlayer++){
         if(pk_minipads[jlayer].size() <=0) continue;
	 ExHit* match_pk = NULL;
         double best_diff = 9999;
         int jNpk = pk_minipads[jlayer].size();
	 for(int jpk=0;jpk<jNpk;jpk++){
           ExHit* jpk_pad = pk_minipads[jlayer][jpk];
	   int jpk_key = jpk_pad->GetKey();
	   double jpk_x = mpcex_mapper->get_x(jpk_key);
	   double jpk_y = mpcex_mapper->get_y(jpk_key);
	   double jpk_z = mpcex_mapper->get_z(jpk_key);
	   double jpk_hsx = jpk_x/(jpk_z - _vertex);
	   double jpk_hsy = jpk_y/(jpk_z - _vertex);
	   double diff_hsx = fabs(jpk_hsx - pk_hsx);
	   double diff_hsy = fabs(jpk_hsy - pk_hsy);
           double comp_diff = diff_hsx;
           int jpk_index = get_index(jpk_pad);
	   double x_length = 0;
	   double y_length = 0;
	     //same type layer
	   if((ilayer - jlayer)%2 == 0){
	     if(ilayer%2==0){
               if(used_list_x.find(jpk_index)!=used_list_x.end()) continue;
               used_list_x.insert(jpk_index);
	     }
	     if(ilayer%2==1){
               if(used_list_y.find(jpk_index)!=used_list_y.end()) continue;
               used_list_y.insert(jpk_index);
	     }
	       //x type layer
	     x_length = pad_short;
	     y_length = pad_long;
	       //y type layer
	     if(ilayer%2==1){
	       x_length = pad_long;
	       y_length = pad_short;
	       comp_diff = diff_hsy;
	     }
	   }
           else{
             if(jlayer%2==0) used_list_x.insert(jpk_index);
	     else used_list_y.insert(jpk_index);
	     x_length = pad_long;
	     y_length = pad_long;
	     comp_diff = sqrt(pow(diff_hsx,2)+pow(diff_hsy,2));
	   }
	     
	   //x type layer, refer x 
	   if(diff_hsx < x_length/fabs(jpk_z-_vertex) && diff_hsy < y_length/fabs(jpk_z-_vertex)){
	     if(comp_diff < best_diff){
               best_diff = diff_hsx;
	       match_pk = jpk_pad;
	     }
	   }
	 }//jpk
	 if(match_pk){
           track->push_back(match_pk);
	 }
       }//jlayer
         
       if(track->size() > 2){
	 _track_lists.push_back(track);
       }
       else delete track;
     }//ipk
   }//ilayer
  
  clean_tracks();
//  PrintTracks(2);
  
}

double STFinderV2::GetdPks(){
  //for choose two highest energy
  //
  int ntracks = _track_lists.size();
  if(ntracks <=0) return -9999;
  //record total energy of tracks
  double tracks_e[2000] = {0.};
  double weight[8] = {1.1,1.05,1.0,0.9,0.9,0.9,0.9,0.9};
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  for(int i=0;i<ntracks;i++){
    Track* track = _track_lists[i];
    int nhits = track->size();
    if(nhits <4) continue; 
    for(int j=0;j<nhits;j++){
      ExHit* hit = track->at(j);
      int key = hit->GetKey();
      int layer = mpcex_mapper->get_layer(key);
      double e = hit->GetE();
      tracks_e[i] += e*weight[layer];
    }
  }
  
  //find the max one
  double max_e = -9999;
  int max_index = -9999;
  for(int i=0;i<ntracks;i++){
    if(tracks_e[i] >0.00001 && tracks_e[i] > max_e ){
      max_index = i;
      max_e = tracks_e[i];
    }
  }
  //no max track found
  if(max_index < 0) return -9999;
  //find second max track
  double next_max_e = -9999;
  int next_max_index = -9999;
  for(int i=0;i<ntracks;i++){
    if(i == max_index) continue;
    if(tracks_e[i] > 0.00001 && tracks_e[i] > next_max_e){
      next_max_index = i;
      next_max_e = tracks_e[i];
    }
  }

  if(next_max_index<0) return -9999;
 
  //find the distance
  int want_tracks[2] = {max_index,next_max_index};
  double m_x[2] = {0.,0.};
  double m_y[2] = {0.,0.};
  double norm_x[2] = {0.,0.};
  double norm_y[2] = {0.,0.};
  for(int j=0;j<2;j++){
    Track* track = _track_lists[want_tracks[j]];
    int nhits = track->size();
    for(int i=0;i<nhits;i++){
      ExHit* hit = track->at(i);
      int key = hit->GetKey();
      double x = mpcex_mapper->get_x(key);
      double y = mpcex_mapper->get_y(key);
      double e = hit->GetE();
      int layer = mpcex_mapper->get_layer(key);
      double scale_x = 1;
      double scale_y = 1/8.;
      if(layer%2==1){
        scale_x = 1/8.;
        scale_y = 1;
      }
      m_x[j] += x*e*scale_x;
      norm_x[j] += e*scale_x;
      m_y[j] += y*e*scale_y;
      norm_y[j] += e*scale_y;
    }
    m_x[j] = m_x[j]/norm_x[j];
    m_y[j] = m_y[j]/norm_y[j];
  }
  
  double dx = m_x[0] - m_x[1];
  double dy = m_y[0] - m_y[1];

  return sqrt(dx*dx+dy*dy);
}

double STFinderV2::GetMass(){
  double dz = 205 - _vertex;
  double dpeaks = GetdPks();
  if(dpeaks < 0) return -9999;
  double xxx = 0.5*dpeaks/dz;
  double cosAngle = (1-xxx*xxx)/(1+xxx*xxx);
  //assumes an asymmetry of alpha = 0.5 so we divide the energy equally between the photons
  double pi0_mass = std::sqrt(0.5*_shower_e*_shower_e*(1-cosAngle));
  

  return pi0_mass;
}

void STFinderV2::clean_tracks(){
  int ntracks = _track_lists.size();
  if(ntracks <=0) return;
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  for(int i=0;i<ntracks;i++){
    Track* track = _track_lists[i];
    double m_x = 0;
    double m_y = 0;
    double weight_x = 0;
    double weight_y = 0;
    int nhits = track->size();
    for(int j=0;j<nhits;j++){
      ExHit* hit = track->at(j);
      int key = hit->GetKey();
      double x = mpcex_mapper->get_x(key);
      double y = mpcex_mapper->get_y(key);
      double z = mpcex_mapper->get_z(key);
      double e = hit->GetE();
//      e = 1;
      double hsx = x/(z-_vertex);
      double hsy = y/(z-_vertex);
      int layer = mpcex_mapper->get_layer(key);
      double scale_x = 1;
      double scale_y = 1./8.;
      if(layer%2==1){
        scale_x = 1./8.;
	scale_y = 1.;
      }
      m_x += hsx*e*scale_x;
      weight_x += e*scale_x;
      m_y += hsy*e*scale_y;
      weight_y += e*scale_y;
    }
    
    m_x = m_x/weight_x;
    m_y = m_y/weight_y;
    double pad_short = MpcExConstants::MINIPAD_SHORT_LENGTH*1.5;
    double pad_long = MpcExConstants::MINIPAD_LONG_LENGTH*0.6;

    vector<ExHit*>::iterator it = track->begin();
    while(it!=track->end()){
      ExHit* hit = *it;
      int key = hit->GetKey();
      double x = mpcex_mapper->get_x(key);
      double y = mpcex_mapper->get_y(key);
      double z = mpcex_mapper->get_z(key);
      double hsx = x/(z-_vertex);
      double hsy = y/(z-_vertex);
      int layer = mpcex_mapper->get_layer(key);
      double thresh_x = pad_short/(z-_vertex);
      double thresh_y = pad_long/(z-_vertex);
      if(layer%2==1){
        thresh_x = pad_long/(z-_vertex);
	thresh_y = pad_short/(z-_vertex);
      }
      if(fabs(hsx-m_x)>fabs(thresh_x) || fabs(hsy-m_y)> thresh_y) track->erase(it); 
      else it++;
    }
  }

}

void STFinderV2::PrintTracks(int N){
  cout<<"Number of tracks: "<<_track_lists.size()<<endl;
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  for(unsigned int i=0;i<_track_lists.size();i++){
    Track track = *(_track_lists[i]);
    if(int(track.size()) < N) continue;
    cout<<"Track "<<i<<endl;
    for(unsigned int j=0;j<track.size();j++){
      ExHit* hit = track[j];
      int key = hit->GetKey();
      double x = mpcex_mapper->get_x(key);
      double y = mpcex_mapper->get_y(key);
      int layer = mpcex_mapper->get_layer(key);
      cout<<key<<" "
          <<layer<<" "
          <<x<<" "
	  <<y<<" "
	  <<hit->GetE()<<" "
	  <<endl;
    }
  }
}
