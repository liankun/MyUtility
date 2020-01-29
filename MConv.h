#ifndef __MCONV_H__
#define __MCONV_H__

/**
 * Convolutional ND, n dimensional
 * **/

#include "MLayer.h"
#include <vector>
#include "MTensor.h"
#include "MIndexing.h"

class MConv: public MLayer{
  private:
    //use the tensor as the filters
    //this is an array of filters
    std::vector<MTensor*> _filters;
    std::vector<double> _bias;
    
    //the padding shift value: added columns
    //and rows for each dimension 
    std::vector<unsigned int> _padding_shift0;
    std::vector<unsigned int> _padding_shift1;
    unsigned int _nft;
    //the shape of the filter
    MShape _shape;
    bool _same_pad;
    unsigned int _stride;
    unsigned int _ft_volume;

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
    MConv(const MShape& shape,
            unsigned int nft,
	    unsigned int stride = 1,
	    bool same_pad=false,
	    bool set_sparse=false,
	    bool for_test = false,
	    double fill_value=0);
    
    virtual ~MConv();
    MTensor* GetOutPut(MTensor*,bool set_sparse=false);
    MTensor* GetFilter(unsigned int i);
    
    //sparse version product
    MTensor* GetOutPutTest(MTensor*,bool set_sparse=true);

    void Print();
    //set the values for filter
    //all values are flatten to 1D
    void SetFilter(const double* values);
    void SetWeights(const double* values);
    //set the values for bias
    void SetBias(const double* values);

    void SetFilter(const std::vector<double>& values);
    void SetWeights(const std::vector<double>& values);
    void SetBias(const std::vector<double>& values);

    //for sparse matrix multiplication
    //for the input index, get the first convolve index
    //it also return a MShape used to loop all possible
    //index
    MIndex GetInitIndex(const MIndex&,const MShape&,MShape&);

};

#endif /**__MCONV_H__**/
