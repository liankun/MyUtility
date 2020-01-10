#ifndef __MMODELV1_H__
#define __MMODELV1_H__

#include "MModelBase.h"
class TMpcExShower;
class ExShower;
class MLayer;
class MPreprocess;
class MTensor;

class MModelV1:public MModelBase
{
  public:
    MModelV1();
    virtual ~MModelV1();
    
    std::vector<double> GetProb(TMpcExShower* shower);
    std::vector<double> GetProb(ExShower* shower);
    std::vector<double> GetProb(MTensor* tensor);

    void Print();
  private:
    std::vector<MLayer*> _mlayers;
    MPreprocess* _prep;
    
};

#endif /**__MMODELV1_H__**/
