#ifndef __MCONVND_H__
#define __MCONVND_H__

/**
 * Convolutional ND, n dimensional
 * **/

#include "MLayer.h"
#include <vector>
#include "MTensor.h"
#include "MIndexing.h"

class MConvND:MLayer{
  private:
    //use the tensor as the filters
    //this is an array of filters
    std::vector<MTensor*> _filters;
    std::vector<float> _bias;

    //the padding shift value: added columns
    //and rows for each dimension 
    std::vector<unsigned int> _padding_shift0;
    std::vector<unsigned int> _padding_shift1;
    unsigned int _nft;
    //the shape of the filter
    MShape _shape;
    bool _same_pad;
    unsigned int _stride;

    MTensor* GetOutTensor(const MShape &shape,bool set_sparse=false);
    void SetPaddingShift(const MShape &shape);
    //check if the index is the input tensor
    //range and return the modified index
    bool GetPaddingIndex(MIndex &index,const MShape &shape);

  public:
    //if for_test is true
    //the fill value will be valid
    //otherwise, the fill valid will
    //be invalid
    //the sparse is set for the filter
    MConvND(const MShape& shape,
            unsigned int nft,
	    unsigned int stride = 1,
	    bool same_pad=false,
	    bool set_sparse=false,
	    bool for_test = false,
	    float fill_value=0);
    
    virtual ~MConvND();
    MTensor* GetOutPut(MTensor*,bool set_sparse=false);
    void Print();

};

#endif /**__MCON2D_H__**/
