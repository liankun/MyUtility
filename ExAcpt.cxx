#include "ExAcpt.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <MpcMap.h>
#include <string>

using namespace std;

ExAcpt* ExAcpt:: _instance = NULL;


ExAcpt* ExAcpt::instance(){
  if(_instance == NULL){
    _instance = new ExAcpt();
    }
  return _instance;
}

//only search -Pi/4 to +Pi/4 range
double ExAcpt::get_shortest_d(int arm,int layer,double x,double y){
  double phi = atan2(y,x)+3.141593;
  int iphi = floor(phi/_dphi);
  double r = sqrt(x*x+y*y);  
  double rmin = _rmin[arm][layer][iphi];
  double rmax = _rmax[arm][layer][iphi];
  if(r>rmax || r<rmin) return -9999;
  double dr = (rmax - rmin)/200;
  int ir = (r - rmin)/dr;
  return _shortest[arm][layer][iphi][ir];
}

bool ExAcpt::is_on_mpcex(int arm,int layer,double x,double y){
  double phi = atan2(y,x)+3.141593;
  int iphi = floor(phi/_dphi);
  double r = sqrt(x*x+y*y);
  if(r >= _rmin[arm][layer][iphi] && r<=_rmax[arm][layer][iphi]){ 
    return true;
  }
  else{
//    cout<<r<<" "<<_rmin[arm][layer][iphi]<<" "<<_rmax[arm][layer][iphi]<<endl;
    return false;
  }
}

bool ExAcpt::is_track_in_mpcex(int arm,double vertex,double hsx,double hsy){
  for(unsigned int ilayer =0;ilayer<8;ilayer++){
    double x = hsx*(_layer_dst[arm][ilayer]-vertex);
    double y = hsy*(_layer_dst[arm][ilayer]-vertex);
//    cout<<"layer "<<ilayer<<endl;
    if(!is_on_mpcex(arm,ilayer,x,y)) return false;
  }
  return true;
}

bool ExAcpt::is_shower_in_mpcex(int arm,double vertex,double hsx,double hsy,double hsrms){
  for(unsigned int ilayer=0;ilayer<8;ilayer++){
    double x = hsx*(_layer_dst[arm][ilayer]-vertex);
    double y = hsy*(_layer_dst[arm][ilayer]-vertex);
    double r = hsrms*(_layer_dst[arm][ilayer]-vertex);
    r = fabs(r);
    if(!is_on_mpcex(arm,ilayer,x,y)) return false; 
    double short_d = get_shortest_d(arm,ilayer,x,y);
    if(r > short_d) return false;
  }
  return true;
}

bool ExAcpt::is_on_mpc(int arm,double x,double y){
  double phi = atan2(y,x)+3.141593;
  int iphi = floor(phi/_dphi);
  double r = sqrt(x*x+y*y);
  if(r >= _mpc_rmin[arm][iphi] && r<=_mpc_rmax[arm][iphi]){
      return true;
  }
  else return false;
}

bool ExAcpt::is_track_in_mpc(int arm,double vertex,double hsx,double hsy){
  double x = (_mpc_front_dst[arm]-vertex)*hsx;
  double y = (_mpc_front_dst[arm]-vertex)*hsy;
  if(is_on_mpc(arm,x,y)) return true;
  else return false;
}

bool ExAcpt::is_shower_in_mpc(int arm,double vertex,double hsx,double hsy,double hsrms){
  double x = (_mpc_front_dst[arm]-vertex)*hsx;
  double y = (_mpc_front_dst[arm]-vertex)*hsy;
  double r = hsrms*(_mpc_front_dst[arm]-vertex);
  r = fabs(r);
  if(!is_on_mpc(arm,x,y)) return false;
  double short_d = get_shortest_d_mpc(arm,x,y);
  if(r > short_d) return false;
  return true;
}

double ExAcpt::get_shortest_d_mpc(int arm,double x,double y){
  double phi = atan2(y,x)+3.141593;
  int iphi = floor(phi/_dphi);
  double r = sqrt(x*x+y*y);
  double rmin = _mpc_rmin[arm][iphi];
  double rmax = _mpc_rmax[arm][iphi];
  if(r < rmin || r > rmax) return -9999;
  
  double dr = (rmax - rmin)/200;
  int ir = (r - rmin)/dr;
  return _mpc_shortest[arm][iphi][ir];
}

ExAcpt::ExAcpt(){
  _dphi = 2*3.141593/1080;
  _layer_dst[0][0] = -203.982;
  _layer_dst[0][1] = -204.636;
  _layer_dst[0][2] = -205.29;
  _layer_dst[0][3] = -205.944;
  _layer_dst[0][4] = -206.598;
  _layer_dst[0][5] = -207.252;
  _layer_dst[0][6] = -207.906;
  _layer_dst[0][7] = -208.56;
  _layer_dst[1][0] = 203.982;
  _layer_dst[1][1] = 204.636;
  _layer_dst[1][2] = 205.29;
  _layer_dst[1][3] = 205.944;
  _layer_dst[1][4] = 206.598;
  _layer_dst[1][5] = 207.252;
  _layer_dst[1][6] = 207.906;
  _layer_dst[1][7] = 208.56;


  _mpc_front_dst[0] = -220.9;
  _mpc_front_dst[1] = 220.9;
   
  int arm;
  int layer;
  int phi;
  double ex_max;
  double ex_min;
  
  std::string file_location = "/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MyUtility/install/share/MyUtility/ExAcpt_data_file_mpcex.txt";
  std::ifstream in_txt(file_location.c_str());
  if(in_txt.is_open()){
    while(in_txt>>arm>>layer>>phi>>ex_max>>ex_min){
      _rmax[arm][layer][phi] = ex_max;
      _rmin[arm][layer][phi] = ex_min;
    }
    in_txt.close();
  }
  else{
    cout<<"Open ExAcpt_data_file_mpcex.txt failed !!"<<endl;
    cout<<"The result is useless !!!"<<endl;
  }
  
  file_location ="/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/MyUtility/install/share/MyUtility/ExAcpt_data_file_mpc.txt" ;
  in_txt.open(file_location.c_str());
  if(in_txt.is_open()){
    while(in_txt>>arm>>phi>>ex_max>>ex_min){
      _mpc_rmax[arm][phi] = ex_max;
      _mpc_rmin[arm][phi] = ex_min;
    }
  }
  else{
    cout<<"Open ExAcpt_data_file_mpc.txt failed !!"<<endl;
    cout<<"The result is useless !!!"<<endl;
  }

   InitRShort();
}

void ExAcpt::InitRShort(){
  //for mpcex
  for(int iarm=0;iarm<2;iarm++){
    for(int ilayer=0;ilayer<8;ilayer++){
      for(int iphi=0;iphi<1080;iphi++){
        double r_min = _rmin[iarm][ilayer][iphi];
        double r_max = _rmax[iarm][ilayer][iphi];
        double dr = (r_max - r_min)/200;
      
        for(int ir=0;ir<200;ir++){
          double r = r_min + dr*ir;
          double dst=9999;
	  for(int i=iphi-135;i<=iphi+135;i++){
            int k=i;
            if(i<0) k=i+1080;
            if(i>=1080) k=i-1080;
            double ang = _dphi*(i-iphi);
            double rmin = _rmin[iarm][ilayer][k];
            double rmax = _rmax[iarm][ilayer][k];
            double r0 = sqrt(r*r+rmin*rmin-2*r*rmin*cos(ang)+0.0000001);
            double r1 = sqrt(r*r+rmax*rmax-2*r*rmax*cos(ang)+0.0000001);
            if(dst > r0) dst = r0;
            if(dst > r1) dst = r1;
          }
          _shortest[iarm][ilayer][iphi][ir] = dst; 
        }
      }
    }
  }
  
  for(int iarm=0;iarm<2;iarm++){
    for(int iphi=0;iphi<1080;iphi++){
      double r_min = _mpc_rmin[iarm][iphi];
      double r_max = _mpc_rmax[iarm][iphi];
      double dr = (r_max - r_min)/200;
      for(int ir=0;ir<200;ir++){
        double r = r_min + dr*ir;
        double dst=9999;
        for(int i=iphi-135;i<=iphi+135;i++){
          int k=i;
          if(i<0) k=i+1080;
          if(i>=1080) k=i-1080;
          double ang = _dphi*(i-iphi);
          double rmin = _mpc_rmin[iarm][k];
          double rmax = _mpc_rmax[iarm][k];
          double r0 = sqrt(r*r+rmin*rmin-2*r*rmin*cos(ang)+0.0000001);
          double r1 = sqrt(r*r+rmax*rmax-2*r*rmax*cos(ang)+0.0000001);
          if(dst > r0) dst = r0;
          if(dst > r1) dst = r1;
        }
        _mpc_shortest[iarm][iphi][ir] = dst; 
      }
    }
  }
}

void ExAcpt::print_data(const int opt){
  if(opt == 0){
    cout<<"Print MpcEx: "<<endl;
    for(int iarm=0;iarm<2;iarm++){
      for(int ilayer=0;ilayer<8;ilayer++){
        for(int iphi = 0;iphi<1080;iphi++){
          cout<<iarm<<" "<<ilayer<<" "<<iphi<<" "
	      <<_rmax[iarm][ilayer][iphi]<<" "
	      <<_rmin[iarm][ilayer][iphi]
	      <<endl;
	}
      }
    }
  }

  if(opt == 1){
    cout<<"Print Mpc: "<<endl;
    for(int iarm=0;iarm<2;iarm++){
      for(int iphi=0;iphi<1080;iphi++){
        cout<<iarm<<" "<<iphi<<" "
	    <<_mpc_rmax[iarm][iphi]<<" "
	    <<_mpc_rmin[iarm][iphi]<<endl;
	    
      }
    }
  }
}
