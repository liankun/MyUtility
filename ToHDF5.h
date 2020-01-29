#ifndef __TOHDF5__H__
#define __TOHDF5__H__
/**
 *Transfer TMpcExShower or ExShower
 *to a file of HDF5
 *H5Cpp.h can not be parsed by the 
 *rootcint, This class can not be used
 *by the root prompt
 * **/
//#include <H5Cpp.h>
#include <string>

//important, or it will conflict
//with the Root CINT
namespace H5 {
  class H5File;
  class DataSet;
}

//dimension for the array
//the array can extendable
//in N0 direction
const int RANK = 4;
const int N0=1;
const int NX=65;
const int NY=65;
const int NZ=8;


class ExShower;
class TMpcExShower;

class ToHDF5{
  private:
    H5::H5File* _h5;
    H5::DataSet* _data_set;
    float _data[N0][NX][NY][NZ];
    unsigned int  _entries;
    unsigned int _compress;
    void ResetData();

  public:
    ToHDF5(std::string name="Ex.h5",std::string data_set_name="data_set");
    virtual ~ToHDF5();
    bool FillDataSet(ExShower* shower);
    void FillDataSet(TMpcExShower* shower){}
    bool SetArrayByHit(int layer,double x,double y,double e);
    void SetCompressLevel(unsigned int val) {_compress = val;}
};

#endif
