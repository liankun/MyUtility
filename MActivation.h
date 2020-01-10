#ifndef __MACTIVATION_H__
#define __MACTIVATION_H__

#include "MLayer.h"

class MActivation : public MLayer
{
  public:
    enum Function {
      RELU=0,
      SIGMOID=1,
      SOFTMAX=2,
    };

    MActivation(Function fc=RELU);
    virtual ~MActivation(){}
    
    MTensor* GetOutPut(MTensor* tensor,bool set_sparse=false);

    void Print();
    double GetActValue(double val);
  
  private:
    Function _fc;
};
#endif
