#ifndef __MINICLUSTERV1__
#define __MINICLUSTERV1__

#include "ExMiniClusters.h"

class MiniClusterV1:MiniCluster
{
  public:
     MiniClusterV1();
     virtual ~MiniClusterV1();

     double GetRMS(){
       double rms = sqrt(_rms_x*_rms_x+_rms_y*_rms_y+1.0e-10);
       if(rms<=1.0e-4) rms=0;
       return rms;
     }
     double GetRMSX() {return _rms_x;}
     double GetRMSY() {return _rms_y;}
     double GetRadius() {return _radius;}
     double GetPkE();
     double GetExE();
   
     unsigned int GetGridRange(){return _sq_nxn_e.size();}
     double GetSqENxN(unsigned int i) {if(i<_sq_nxn_e.size()) return _sq_nxn_e[i];return -9999;}

     double GetLayerPkE(unsigned int i) {if(i<8) return _layer_pk_e[i];return -9999;}
     double GetLayerE(unsigned int i) {if(i<8) return _layer_ex_e[i];return -9999;}
     unsigned int GetNSq(){return _sq_list.size();}

     Square* GetSq(unsigned int i) {if(i<_sq_list.size()) return _sq_list[i];return NULL;} 
   
     double GetX() {return _x;}
     double GetY() {return _y;}

     double GetPkX() {return _pk_x;}
     double GetPkY() {return _pk_y;}

     int GetPkGridX() {return _pk_ix;}
     int GetPkGridY() {return _pk_iy;}


     bool IsSeedPk() {return _is_seed_pk;}

     void SetRMSX(double value) {_rms_x = value;}
     void SetRMSY(double value) {_rms_y = value;}
     void SetRadius(double val) {_radius = val;}
     void SetLayerPkE(unsigned int i,double val) {if(i<8) _layer_pk_e[i] = val;}
     void SetLayerE(unsigned int i,double val) {if(i<8) _layer_ex_e[i] = val;}

     void SetX(double val) {_x = val;}
     void SetY(double val) {_y = val;}
     void SetPkX(double val) {_pk_x = val;}
     void SetPkY(double val) {_pk_y = val;}

     void SetPkGridX(int val) {_pk_ix = val;}
     void SetPkGridY(int val) {_pk_iy = val;}


     void SetIsSeedPk(bool val) {_is_seed_pk = val;}

     void InsertSq(Square* sq ){_sq_list.push_back(sq);}
     void InsertSqENxN(double val) {_sq_nxn_e.push_back(val);}

     virtual void Reset();
     void Print();

     virtual MiniCluster* Clone();


  private:
    double _rms_x;
    double _rms_y;
    double _pk_x;//pk may not exist
    double _pk_y;//
    int _pk_ix; //grid pk ix
    int _pk_iy; //grid pk iy
    double _x;//mean value
    double _y;
    double _radius;
    double _layer_pk_e[8];
    double _layer_ex_e[8];
    bool _is_seed_pk;
    std::vector<Square*> _sq_list;
    std::vector<double> _sq_nxn_e;

};

#endif
