#ifndef __MFLATTEN_H__
#define __MFLATTEN_H__

#include "MLayer.h"

class MTensor;

class MFlatten : public MLayer
{
  private:

  public:
    MFlatten(){MLayer();}
    virtual ~MFlatten(){}
    MTensor* GetOutPut(MTensor* tensor,bool set_sparse = false);
    void Print();
};

#endif /**__MFLATTEN_H__**/
