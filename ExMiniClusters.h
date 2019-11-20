#ifndef __EXMINICLUSTERS__
#define __EXMINICLUSTERS__

#include <stdio.h>
#include <iostream>
#include "ExEvent.h"


#define WHERE __LINE__

class TMpcExShower;
class TMpcExHitContainer;


class Square{
  private:
    double _x;
    double _y;
    //grid index
    int _ix;
    int _iy;
    double _layer_e[8];
  public:
    
    Square(){_x=0;_y=0;memset(_layer_e,0,sizeof(_layer_e));_ix=0;_iy=0;}
    Square(double x,double y,double layer_e[8]){
      _x = x;
      _y = y;
      for(int i=0;i<8;i++){
        _layer_e[i] = layer_e[i]; 
      }
      _ix = 0;
      _iy = 0;
    }

    double GetX() {return _x;}
    double GetY() {return _y;}
    int GetGridX() {return _ix;}
    int GetGridY() {return _iy;}
    double GetE(){
      double sum=0;
      for(int i=0;i<8;i++){
        sum += _layer_e[i];
      }
      return sum;
    }

    double GetLayerE(unsigned int i) {if(i<8) return _layer_e[i];return 0;}

    void Print();

    void SetX(double val) {_x = val;}
    void SetY(double val) {_y = val;}
    void SetGridX(int val) {_ix = val;}
    void SetGridY(int val) {_iy = val;}
    void SetLayerE(unsigned int i,double val){if(i<8) _layer_e[i] = val;}

    Square* Clone();
};



class MiniCluster
{
  public:
    
    
    MiniCluster(){}
    virtual ~MiniCluster(){}
    
    virtual double GetRMSX() {std::cout<<WHERE<<" GetRMSX "<<std::endl;return -9999;}
    virtual double GetRMSY() {std::cout<<WHERE<<" GetRMSY "<<std::endl;return -9999;}
    virtual double GetRMS() {std::cout<<WHERE<<" GetRMS "<<std::endl;return -9999;}
    virtual double GetRadius() {std::cout<<WHERE<<" GetRadius "<<std::endl;return -9999;}
    virtual double GetPkE() {std::cout<<WHERE<<" GetPKE "<<std::endl;return -9999;}
    virtual double GetExE() {std::cout<<WHERE<<" GetExE "<<std::endl;return -9999;}
    //get the largest grid range, the max size of nxn
    virtual unsigned int GetGridRange() {std::cout<<WHERE<<" GetGridRange  "<<std::endl;return 0;} 
    //energy of nxn square energy 
    virtual double GetSqENxN(unsigned int n) {std::cout<<WHERE<<" GetSqENxN  "<<std::endl;return 0;} 

    //we can seperate MpcE for this miniCluster in future
    virtual double GetMpcE() {std::cout<<WHERE<<" GetMpcE "<<std::endl;return -9999;}
    
    //get closest Mpc Tower Energy for this miniCluster
    virtual double GetTowerE() {std::cout<<WHERE<<" GetTowerE  "<<std::endl;return -9999;}
    
    //get closest Mpc Tower Channel
    virtual int GetTowerCh() {std::cout<<WHERE<<" GetTowerCh   "<<std::endl;return -9999;}
    //we can combine MpcE and TotalE in future
    virtual double GetTotE() {std::cout<<WHERE<<" GetTotE "<<std::endl;return -9999;}
    
    //get the total shower energy of which the minicluster contains
    virtual double GetShowerE() {std::cout<<WHERE<<" GetShowerE "<<std::endl;return -9999;}
    //vertex
    virtual double GetVertex() {std::cout<<WHERE<<" GetVertex "<<std::endl;return -9999;}

    virtual double GetShowerX() {std::cout<<WHERE<<" GetShowerX "<<std::endl;return -9999;}

    virtual double GetShowerY() {std::cout<<WHERE<<" GetShowerY "<<std::endl;return -9999;}

    //get Layer Peak E
    virtual double GetLayerPkE(unsigned int layer) {std::cout<<WHERE<<" GetLayerPkE "<<std::endl;return -9999;}
    virtual double GetLayerE(unsigned int layer) {std::cout<<WHERE<<" GetLayerE "<<std::endl;return -9999;}
    //return position of the Peak X
    virtual double GetPkX() {std::cout<<WHERE<<" GetPkX "<<std::endl;return -9999;}
    virtual double GetPkY() {std::cout<<WHERE<<" GetPkY "<<std::endl;return -9999;}

    virtual int GetPkGridX() {std::cout<<WHERE<<" GetPkGridX "<<std::endl;return -9999;}
    virtual int GetPkGridY() {std::cout<<WHERE<<" GetPkGridY "<<std::endl;return -9999;}


    virtual double GetX() {std::cout<<WHERE<<" GetX "<<std::endl;return -9999;}
    virtual double GetY() {std::cout<<WHERE<<" GetY "<<std::endl;return -9999;}

    virtual bool IsSeedPk() {std::cout<<WHERE<<" IsSeedPk "<<std::endl;return false;}

    //cluster is represent by a 2d array nxn square box
    virtual unsigned int GetNSq(){std::cout<<WHERE<<" GetNSq  "<<std::endl;return 0;}
    virtual Square* GetSq(unsigned int) {std::cout<<WHERE<<" GetSq   "<<std::endl;return NULL;}

    virtual void Reset(){std::cout<<WHERE<<" Reset    "<<std::endl;}

    virtual void Print() {std::cout<<WHERE<<" Print     "<<std::endl;}
    virtual MiniCluster* Clone() {std::cout<<WHERE<<" Clone     "<<std::endl;return NULL;}

  private:
    /*this is a base class*/
    
};

class ExMiniClusters{
  public:
     ExMiniClusters();
     ExMiniClusters(ExShower* );
     ExMiniClusters(TMpcExShower*,TMpcExHitContainer* );
    
     virtual void Reset();
     virtual ~ExMiniClusters();

     unsigned int GetNMiniCluster();
     MiniCluster* GetMiniCluster(unsigned int i);
     
     //construct MiniClusters
     void ConstructMiniClusters(ExShower*);
     void ConstructMiniClusters(TMpcExShower*,TMpcExHitContainer*);

     void SetDebug(bool val=true){
       _debug = val;
     }
     
     void SetThresholdRMS(double val) {_th_rms = val;}
     void SetThresholdRMSAsy(double val) {_th_rms_asy = val;}
     void SetThresholdPKMeanDST(double val) {_th_pk_mean_dist = val;}
     void SetMaxIterate(int n) {_max_iterate = n;}

     void SetSqCuts(bool val) {_sq_cuts = val;}
     //in keV
     void SetMinipadThresholdE(double val) {_th_mpad_e=val;}


     void VisualMiniClusters(ExShower* ex_shower,const char* data_set="");

  private:
     std::vector<MiniCluster*> _mini_cluster_list;
     //make a cut on Square, these Square will not
     //used in miniCluster process
     std::vector<Square*> _delete_sq_list;

     //flag for sq cuts 
     bool _sq_cuts;
     
     //flage for debug, the initial value will be true
     bool _debug;
     //thershold for clustering
     //the default value is 1
     double _th_rms;
     //max number iteration for miniclusters
     //the defualt is 100; which is the size 
     //of the MpcEx
     //ratio of (rms_x-rms_y)/(rms_x+rms_y)
     //the idea case is 0 when rms_x ~ rms_y
     //the default value will be 0.5 , when rms_x/rms_y~3
     double _th_rms_asy;
     
     //the distance between pk square and mean x y of cluster
     //we use the rms as a reference
     double _th_pk_mean_dist;

     double _th_mpad_e;

     int _max_iterate;
     
     void print_square_2d(std::vector<Square*> sq_list,ExShower* ex_shower);

     bool IsGoodSq(Square* sq);

};


#endif
