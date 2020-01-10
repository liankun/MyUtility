#ifndef __MMODELBase_H__
#define __MMODELBase_H__

#include <vector>

class TMpcExShower;
class ExShower;

class MModelBase{
  public:
    MModelBase();
    virtual ~MModelBase();
    //output an array, each one is the probability for each type 
    virtual std::vector<double> GetProb(TMpcExShower*);
    virtual std::vector<double> GetProb(ExShower*);
    //Print the model information
    virtual void Print();

  private:

};

#endif /**__MMODEL_H__**/
