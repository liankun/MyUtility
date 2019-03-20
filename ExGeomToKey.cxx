#include "ExGeomToKey.h"
#include <iostream>
#include <MpcExMapper.h>
using namespace std;

ExGeomToKey* ExGeomToKey::_instance = NULL;

int ExGeomToKey::get_key(int arm,int layer,double x,double y){
  //avoid cutting edge 
  double dx = _x_type_x_width/2.*1.0001;
  double dy = _x_type_y_width/2.*1.0001;
  double s_x = x;
  double s_y = y;

  if(layer%2==1){
    s_x = y;
    s_y = x;
  }
  
//  cout<<"dx "<<dx<<" dy "<<dy<<endl;
  //An iterator to the the first element in the container 
  //whose key is not considered to go before k, or map::end 
  //if all keys are considered to go before k.
  //at least not less than the value x
  XYToXYMap::iterator it_begin = _xy_to_xy_map[arm][layer].lower_bound(s_x-dx);
  XYToXYMap::iterator it_end = _xy_to_xy_map[arm][layer].lower_bound(s_x+dx);
  
  //there is a situation when begin and end are the same
  if(it_end !=_xy_to_xy_map[arm][layer].end()) it_end++;
  
  for(XYToXYMap::iterator it = it_begin;it!=it_end;++it){
    double ref_x = it->first;
//    cout<<"s_x "<<s_x<<endl;
//    cout<<"ref x "<<ref_x<<endl;
//    cout<<fabs(ref_x-s_x)<<endl;
    if(fabs(ref_x-s_x)<=dx){
//      cout<<"find good ref x "<<ref_x<<endl;
      XYToKey& xy_to_key = it->second;
      XYToKey::iterator t_it_begin = xy_to_key.lower_bound(s_y-dy);
      XYToKey::iterator t_it_end = xy_to_key.lower_bound(s_y+dy);
      
//      cout<<"number minipad: "<<xy_to_key.size()<<endl;

      if(t_it_end!=xy_to_key.end()) t_it_end++;

      for(XYToKey::iterator t_it = t_it_begin;t_it!=t_it_end;++t_it){
        double ref_y = t_it->first;
//	cout<<"s_y "<<s_y<<endl;
//	cout<<"ref y "<<ref_y<<endl;
//	cout<<fabs(ref_y-s_y)<<endl;
	if(fabs(ref_y-s_y)<=dy){
	  return t_it->second;
	}
      }
    }
  }
    
  return -9999;
}

ExGeomToKey::ExGeomToKey(){
  MpcExMapper* ex_mapper = MpcExMapper::instance();
  cout<<"initialize the ExGeomToKey "<<endl;
  for(int ikey=0;ikey<49152;ikey++){
    int arm = ex_mapper->get_arm(ikey);
    int layer = ex_mapper->get_layer(ikey);
    double x = ex_mapper->get_x(ikey);
    double y = ex_mapper->get_y(ikey);
    if(layer%2==0){
      _x_type_x_width = ex_mapper->get_minipad_x_width(ikey);
      _x_type_y_width = ex_mapper->get_minipad_y_width(ikey);
      XYToKey& xy_to_key = _xy_to_xy_map[arm][layer][x];
      xy_to_key.insert(pair<double,int>(y,ikey));
//      cout<<"insert  "<<ikey<<endl;
    }
    else{
      _y_type_x_width = ex_mapper->get_minipad_x_width(ikey);
      _y_type_y_width = ex_mapper->get_minipad_y_width(ikey);
      XYToKey& xy_to_key = _xy_to_xy_map[arm][layer][y];
      xy_to_key.insert(pair<double,int>(x,ikey));
//      cout<<"insert "<<ikey<<endl;
    }
  }
}

ExGeomToKey* ExGeomToKey::instance(){
  if(!_instance) _instance = new ExGeomToKey();
  return _instance;
}

int ExGeomToKey::get_num_keys(){
  int key_num=0;
  for(int iarm=0;iarm<2;iarm++){
    for(int ilayer=0;ilayer<8;ilayer++){
      map<double,XYToKey,own_double_less>::iterator it = _xy_to_xy_map[iarm][ilayer].begin();
      map<double,XYToKey,own_double_less>::iterator end = _xy_to_xy_map[iarm][ilayer].end();
      for(;it!=end;++it){
        XYToKey& xy_to_key = it->second;
	key_num += xy_to_key.size();
      }
    }
  }
  return key_num;
}

void ExGeomToKey::print(){
  for(int iarm=0;iarm<2;iarm++){
    for(int ilayer=0;ilayer<8;ilayer++){
      cout<<"arm "<<iarm<<" layer "<<ilayer<<" "
          <<"first "<<_xy_to_xy_map[iarm][ilayer].begin()->first<<endl;
      map<double,XYToKey,own_double_less>::iterator it = _xy_to_xy_map[iarm][ilayer].begin();
      map<double,XYToKey,own_double_less>::iterator end = _xy_to_xy_map[iarm][ilayer].end();
      for(;it!=end;++it){
        XYToKey& xy_to_key = it->second;
        cout<<"Number of keys "<<xy_to_key.size()<<endl;
      }

    }
  }
}
