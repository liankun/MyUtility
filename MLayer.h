#ifndef __MLAYER__H__
#define __MLAYER__H__

#include<string>
#include"MTensor.h"

/**
 *the base layer of the layer ML
 * **/

class MLayer{
  private:

  public:
    MLayer(){}
    virtual ~MLayer();
    virtual MTensor* GetOutPut(MTensor* tensor,bool set_sparse=false);
    //print the information of the layer
    virtual void Print();
};

#endif /**MLayers **/
