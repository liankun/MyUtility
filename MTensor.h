#ifndef __MTENSOR_H__
#define __MTENSOR_H__

/**
 * This module is designed as the
 * data holder for the ML
 **/

#include <map>
#include <vector>
#include "MIndexing.h"

#define WHERE __LINE__

class MTensor{
  //all tensor will be considered as
  //one D array,access by indexing
  private:
    float* _tensor;
    MSparseMap _sparse_map; 
    MShape _shape;
//    unsigned long int
    //the effective size of the tensor
    //for the sparse map
    unsigned int _nsize;
    //the total volume of the tensor
    unsigned int _volume;
    //the list used to tranfrom the 1D
    //index to multiple index
    std::vector<unsigned int> _tf; 
    bool _is_sparse;
    

    //since array is stored as 1D shape,each 
    //multi-demension is converted to a 1D value
    //if the index is (m,n,s,t), the 1D index will
    //be m+m*n+m*n*s+m*n*s*t
    //the index is on x axis first
    int Get1DIndex(const MIndex& index);

  public:
    //each tensor has its shape (m,n,t,s)
    MTensor(const MShape& shape,bool set_sparse=false);
    //for testing 
    MTensor(){
      _tensor = 0;
      _nsize = 0;
      _is_sparse = false;
    }
    
    virtual ~MTensor();
    //access the element by index
    float GetValue(const MIndex index);
    //access the element by 1D array index;
    float GetValue(const unsigned int i);
    //by using [] the value can be changed
    //for the sparse array
    float &operator[](const MIndex index);
    //access element by index of 1D array
    float &operator[](const unsigned int i);
    bool IsSparse() const {return _is_sparse;}

    void Print1DTensor();

    //check if the index is valid or not
    //each element should <= _shape
    bool IsIndexValid(const MIndex& index);
    //check the shape, all shuold > 0 
    bool IsShapeValid(const MShape& shape);

    unsigned int GetNDim() const {return _shape.size();}
    unsigned int GetShape(unsigned int i) const {if(i<_shape.size()) return _shape[i];return 0;}
    unsigned int GetVolume() const {return _volume;}

    MShape GetShape() const {return _shape;}
    MIndex GetIndexFrom1D(unsigned int);
    
    //set all tensor value to a single value
    //for test
    void SetValue(float val);
    //for test
    void SetValue();

    void SetValue(const unsigned int i,float val);
    void SetValue(const MIndex index,float val);
    
    //all the tensor will be set by one 1D arrays
    //the lenght of the array should be equal to the 
    //Volume of the tensor, or it will cause errors
    //make sure the volume matches
    void Set1DValues(const float* values);
    void Set1DValues(const std::vector<float>& values);
    
    void Set1DValues(std::vector<float>::const_iterator begin,
                     std::vector<float>::const_iterator end);
    void Clear();

};

#endif /**MTensor**/
