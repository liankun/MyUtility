#ifndef __MDENSE_H__
#define __MDENSE_H__

/**
 * implement the fully connect layer
 * input must be a 1D array
 * output will be a 1D array
 * the input shape must be 2D array
 * **/

#include "MLayer.h"
#include "MIndexing.h"
#include <vector>

class MDense:public MLayer
{
  private:
    MTensor* _mat;
    MShape _shape;
    std::vector<double> _bias;
    bool _set_sparse;
    unsigned int _volume;
  public:
    MDense(const MShape& shape,
           bool set_sparse=false); 
    virtual ~MDense();
    MTensor* GetOutPut(MTensor*,bool set_sparse=false);

    void Print();
    void SetWeights(const double* weights);
    void SetWeights(const std::vector<double>& weights);
    void SetBias(const double* bias);
    void SetBias(const std::vector<double>& bias);

};

#endif /**__MDENSE_H__**/
