#ifndef __MPOOLING_H__
#define __MPOOLING_H__

/**
 * Convolutional ND, n dimensional
 * **/

#include "MLayer.h"
#include <vector>
#include "MTensor.h"
#include "MIndexing.h"

class MPooling: public MLayer
{
   public:
     
     enum Method {
      MAX=0,
      AVG=1
     };
     //if for_test is true
     //the fill value will be valid
     //otherwise, the fill valid will
     //be invalid
     //the sparse is set for the filter
     MPooling(const MShape& shape,
	    unsigned int stride = 2,
	    bool same_pad=false,
	    Method md=MAX
	    );
    
     virtual ~MPooling();
     MTensor* GetOutPut(MTensor*,bool set_sparse=false);
     void Print();

  private:
    Method _method;
    //the padding shift value: added columns
    //and rows for each dimension 
    std::vector<unsigned int> _padding_shift0;
    std::vector<unsigned int> _padding_shift1;
    //the shape of the filter
    MShape _shape;
    bool _same_pad;
    unsigned int _stride;
    
    //a dummy tensor used to get the index
    MTensor* _dummy_ft;
    //the pooling layer is about one dimension less than the
    //input tensor

    MTensor* GetOutTensor(const MShape &shape,bool set_sparse=false);
    void SetPaddingShift(const MShape &shape);
    //check if the index is the input tensor
    //range and return the modified index
    bool GetPaddingIndex(MIndex &index,const MShape &shape);
};

#endif /**__MPOOLING_H__**/
