#ifndef __MCONV2D_H__
#define __MCONV2D_H__

/**
 * Convolutional 2D
 * **/

#include "MLayer.h"
#include <vector>
#include "MTensor.h"
#include "MIndexing.h"

class MConv2D:MLayer{
  private:
    //use the tensor as the filters
    //this is an array of filters
    std::vector<MTensor*> _filters;
    unsigned int _nft;
    //the shape of the filter
    MShape _shape;
    bool _same_pad;
    unsigned int _stride;

    MTensor* GetOutTensor(const MShape &shape,bool set_sparse=false);
  public:
    MConv2D(const MShape& shape,
            unsigned int nft,unsigned int stride = 1,bool same_pad=true);
    MConv2D(int mode=0);
    virtual ~MConv2D();
    MTensor* GetOutPut(MTensor*,bool set_sparse=false);
    void Print();



};

#endif /**__MCON2D_H__**/
