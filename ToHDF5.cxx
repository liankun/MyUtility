#include "ToHDF5.h"
#include "ExEvent.h"
#include <MpcExMapper.h>
#include <TMpcExShower.h>
#include <H5Cpp.h>

const double MINSPACE=-20; //20cm
const double DSPACE=40./214.;//in cm
const double SHOWERSIZE=6;//a size of 6cm x 6cm
const int NSPACE=int(2*SHOWERSIZE/DSPACE)+1;

ToHDF5::ToHDF5(std::string name,std::string data_set_name){
  ResetData();
  _entries=0;
  _h5 = new H5::H5File(name,H5F_ACC_TRUNC);
  _compress=6;
  hsize_t dims[RANK];
  dims[0] = N0;
  dims[1] = NX;
  dims[2] = NY;
  dims[3] = NZ;
  
  
  //create the table
  hsize_t max_dims[RANK] = {H5S_UNLIMITED,NX,NY,NZ};
  H5::DataSpace dataspace(RANK,dims,max_dims);
  H5::FloatType datatype(H5::PredType::NATIVE_FLOAT);
  H5::DSetCreatPropList cparms;
  hsize_t      chunk_dims[RANK] ={1,NX,NY,NZ};
  cparms.setChunk(RANK,chunk_dims);
  float fill_val = 0;
  cparms.setFillValue(H5::PredType::NATIVE_FLOAT,&fill_val);
  //compressed level
  cparms.setDeflate(_compress);

  _data_set = new H5::DataSet(_h5->createDataSet(data_set_name,H5::PredType::NATIVE_FLOAT,dataspace,cparms));
 
}

bool ToHDF5::FillDataSet(ExShower* shower){
  //fill the _data with zero
  ResetData();
  MpcExMapper* ex_map = MpcExMapper::instance();
  int nhits = shower->GetNhits();
  double vertex = shower->GetVertex();
  double z=205;
  if(shower->GetArm()==0) z=-205;
  double ct_x = shower->GetHsx()*(z-vertex);
  double ct_y = shower->GetHsy()*(z-vertex);
  
  //GeV->keV
  double raw_ex_e = shower->GetRawMpcexE()*1.0e6;

  for(int i=0;i<nhits;i++){
    ExHit* hit = shower->GetExHit(i);
    int key = hit->GetKey();
    double x = ex_map->get_x(key)-ct_x;
    double y = ex_map->get_y(key)-ct_y;
    double e = hit->GetE()/raw_ex_e;
    int layer = ex_map->get_layer(key);

    bool is_valid=SetArrayByHit(layer,x,y,e);
    if(!is_valid) return false;
  }

  //fill the HDF5 Array
  //extended by one array
  hsize_t extend_size[RANK];
  extend_size[0] = _entries+1;
  extend_size[1] = NX;
  extend_size[2] = NY;
  extend_size[3] = NZ;
  _data_set->extend(extend_size);
  
  //fill position
  hsize_t offset[RANK];
  offset[0] = _entries;
  offset[1] = 0;
  offset[2] = 0;
  offset[3] = 0;

  hsize_t fill_dims[RANK] = {1,NX,NY,NZ};
  H5::DataSpace fill_space = _data_set->getSpace();

  //memory space
  H5::DataSpace mspace(RANK,fill_dims);
  
  //select the read position
  fill_space.selectHyperslab(H5S_SELECT_SET,fill_dims,offset);
  _data_set->write(_data,H5::PredType::NATIVE_FLOAT,mspace,fill_space);

  _entries++;

  return true;
}

bool ToHDF5::SetArrayByHit(int layer,double x,double y,double e){
  int binx0 = int((x+SHOWERSIZE)/DSPACE);
  int binx1 = binx0+1;
  int biny0 = int((y+SHOWERSIZE-DSPACE/2.*7.)/DSPACE);
  int biny1 = int((y+SHOWERSIZE+DSPACE/2.*7.)/DSPACE)+1;

  if(layer%2==1){
    biny0 = int((y+SHOWERSIZE)/DSPACE);
    biny1 = biny0+1;
    binx0 = int((x+SHOWERSIZE-DSPACE/2.*7)/DSPACE);
    binx1 = int((x+SHOWERSIZE+DSPACE/2.*7)/DSPACE)+1;
  }
     
  for(int ix=binx0;ix<binx1;ix++){
    for(int iy=biny0;iy<biny1;iy++){
      if(ix<0||ix>=NSPACE){
          continue;
      }
      if(iy<0||iy>=NSPACE){
	continue;
      }
      _data[0][ix][iy][layer] = e;
//      _data[0][ix][iy][layer] = 0;
    }
  }
  return true;
}


ToHDF5::~ToHDF5(){
  if(_h5) _h5->close();
  _h5 = 0;
  if(_data_set) delete _data_set;
  _data_set=0;
}

void ToHDF5::ResetData(){
  for(unsigned int i=0;i<N0;i++){
    for(unsigned int j=0;j<NX;j++){
      for(unsigned int k=0;k<NY;k++){
	for(unsigned int m=0;m<NZ;m++){
	  _data[i][j][k][m]=0;
	}
      }
    }
  }
}
