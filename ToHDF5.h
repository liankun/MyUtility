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
#include <vector>
#include <map>

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
class TMpcExHitContainer;

typedef std::vector<int> Dims;

class ToHDF5{
  private:
    H5::H5File* _h5;
    //each data set is a table
    std::vector<H5::DataSet*> _ds_list;
    //demension of each one
    std::vector<Dims> _dm_list;
    //pointer to each value
    std::vector<void*> _var_pt_list;
    //data type
    //Only Int or Float
    std::vector<std::string> _dtype_list;
    
    //this will added by default;
    float _data[N0][NX][NY][NZ];
    float _data_sum[N0][NX][NY];
    float _mpcex_raw_e;
    void ResetData();

    //record number of entries
    unsigned int  _entries;
    unsigned int _compress;
    float _ct_x;
    float _ct_y;

    bool IsPeak(int ix,int iy);

  public:
    ToHDF5(std::string name="Ex.h5");
    virtual ~ToHDF5();
    bool Set2DHits(ExShower* shower);
    bool Set2DHits(TMpcExShower* shower,TMpcExHitContainer* hits);
    bool Set2DHits(float data[NX][NY][NZ]);
    bool SetArrayByHit(int layer,double x,double y,double e);
    void SetCompressLevel(unsigned int val) {_compress = val;}
    //only supoort float type
    void AddVar(std::string name,std::string dtype,void* var_pt,const Dims dims);

    //get maximum 2d position 
    //use after Set2DHits
    //the shower center for this is the (shower_x,shower_y)
    //not (0,0)
    void GetMax2DXY(float& m_ix,float& m_iy);

    // check if exist max
    // x,y stands for the shower ct 
    // corrected value
    int GetPeakIndex(float x,float y,float range=0.4); 

    //get the total hit area in 65x65 array
    int GetTotHitArea();

    void Fill();
};

#endif
