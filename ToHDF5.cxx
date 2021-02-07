#include "ToHDF5.h"
#include "ExEvent.h"
#include <MpcExMapper.h>
#include <TMpcExShower.h>
#include <TMpcExHitContainer.h>
#include <map>
#include <H5Cpp.h>
#include <TFile.h>

const double MINSPACE=-20; //20cm
const double DSPACE=40./214.;//in cm
const double SHOWERSIZE=6;//a size of 6cm x 6cm
const int NSPACE=int(2*SHOWERSIZE/DSPACE)+1;


ToHDF5::ToHDF5(std::string name){
  ResetData();
  _entries=0;
  _h5 = new H5::H5File(name,H5F_ACC_TRUNC);
  _compress=6;
  _mpcex_raw_e=0;


  //create dims for 2d hit array
  Dims dims;
  
  //mpcex raw
  AddVar("dp_mpcex_raw_e","Float",&_mpcex_raw_e,dims);
  
  dims.push_back(NX);
  dims.push_back(NY);
  dims.push_back(NZ);
  AddVar("2d_hits","Float",_data,dims);

}

void ToHDF5::AddVar(std::string name,std::string dtype,void* var_pt,const Dims dims){
  if(dtype.compare("Int")!=0 && dtype.compare("Float")!=0){
    std::cout<<"Input Type should be Float or Int !"<<std::endl;
    return;
  }
  
  //the added additional dimentional is the 
  //row number
  unsigned int rank = dims.size()+1;
//  std::cout<<"number of total rank: "<<rank<<std::endl;
  hsize_t hdims[rank];
  hsize_t max_hdims[rank];
  hdims[0]=1;
  //make the first dimension extenable
  max_hdims[0] = H5S_UNLIMITED;
  for(unsigned int i=0;i<dims.size();i++){
    hdims[i+1] = dims[i];
    max_hdims[i+1] = dims[i];
  }
  

  H5::DataSpace dataspace(rank,hdims,max_hdims);
  
  H5::DSetCreatPropList cparms;
  cparms.setChunk(rank,hdims);
  cparms.setDeflate(_compress);
  
  if(dtype.compare("Float")==0){
    float fill_val = 0;
    cparms.setFillValue(H5::PredType::NATIVE_FLOAT,&fill_val);
    H5::DataSet* ds = new H5::DataSet(_h5->createDataSet(name,H5::PredType::NATIVE_FLOAT,dataspace,cparms));
    _ds_list.push_back(ds);
  }

  if(dtype.compare("Int")==0){
    int fill_val = 0;
    cparms.setFillValue(H5::PredType::NATIVE_INT,&fill_val);
    H5::DataSet* ds = new H5::DataSet(_h5->createDataSet(name,H5::PredType::NATIVE_INT,dataspace,cparms));
    _ds_list.push_back(ds);
  }

  _var_pt_list.push_back(var_pt);
  _dm_list.push_back(dims);
  _dtype_list.push_back(dtype);
  
}

bool ToHDF5::Set2DHits(float data[NX][NY][NZ]){
  for(int ix=0;ix<NX;ix++){
    for(int iy=0;iy<NY;iy++){
      for(int iz=0;iz<NZ;iz++){
        _data[0][ix][iy][iz]=data[ix][iy][iz];
      }
    }	  
  }

  return true;
}

bool ToHDF5::Set2DHits(ExShower* shower){
  //fill the _data with zero
  ResetData();
  _mpcex_raw_e=shower->GetRawMpcexE();

  MpcExMapper* ex_map = MpcExMapper::instance();
  int nhits = shower->GetNhits();
  double vertex = shower->GetVertex();
  double z=205;
  if(shower->GetArm()==0) z=-205;
  double ct_x = shower->GetHsx()*(z-vertex);
  double ct_y = shower->GetHsy()*(z-vertex);
  
  _ct_x = ct_x;
  _ct_y = ct_y;
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
  
  //set the sum
  for(int i=0;i<NX;i++){
    for(int j=0;j<NY;j++){
      for(int ilayer=0;ilayer<NZ;ilayer++){
        _data_sum[0][i][j]+=_data[0][i][j][ilayer];
      }	    
    } 
  }

  return true;
}

bool ToHDF5::Set2DHits(TMpcExShower* shower,TMpcExHitContainer* hits){
  ResetData();
  if(!shower || !hits){
    std::cout<<"invalid shower or hits pointer !"<<std::endl;
  }

  _mpcex_raw_e = shower->get_raw_esum();
  double vertex = shower->get_vertex();
  double z=205;
  if(shower->get_arm()==0) z=-205;
  double ct_x = shower->get_hsx()*(z-vertex);
  double ct_y = shower->get_hsy()*(z-vertex);
  
  //GeV->keV
  double raw_ex_e = shower->get_raw_esum()*1.0e6;

  //set the hit for fast look up
  std::map<int,TMpcExHit*> hit_map;
  for(unsigned int i=0;i<hits->size();i++){
    TMpcExHit* hit = hits->getHit(i);
    hit_map[hit->key()]=hit;
  }

  unsigned int nhits = shower->sizeHits();
  for(unsigned int i=0;i<nhits;i++){
    int key = shower->getHit(i);
    TMpcExHit* hit = hit_map[key];
    double x = hit->x()-ct_x;
    double y = hit->y()-ct_y;
    double e = hit->combined()/raw_ex_e;
    int layer = hit->layer();

    bool is_valid=SetArrayByHit(layer,x,y,e);
    if(!is_valid) return false;
  }

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

void ToHDF5::GetMax2DXY(float& m_x,float& m_y){
  double max_e=0;
  for(int i=0;i<NX;i++){
    for(int j=0;j<NY;j++){
      double t_acc_e=0;
      for(int k=0;k<NZ;k++){
        t_acc_e+=_data[0][i][j][k];
      }
      if(t_acc_e>max_e){
        max_e = t_acc_e;
	m_x = i*DSPACE-6+_ct_x-0.5*DSPACE;
	m_y = j*DSPACE-6+_ct_y-0.5*DSPACE;
      }
    }	 
  }
}

int ToHDF5::GetPeakIndex(float x,float y,float range){
  //x and y are shower x and y corrected values
  if(x>6 || x<-6) return -1;
  if(y>6 || y<-6) return -1;
  int bin_x = int((x+SHOWERSIZE)/DSPACE);
  int bin_y = int((y+SHOWERSIZE)/DSPACE);
  int bin_range = int(range/DSPACE);
  //the best peak is the E largest one
  float max_e=0;
  int max_ix = 0;
  int max_iy = -1;
  for(int i=bin_x-bin_range;i<=bin_x+bin_range;i++){
    if(i<0 || i>=65) continue;
    for(int j=bin_y-bin_range;j<=bin_y+bin_range;j++){
      if(j<0 || j>=65) continue;
      //check if this is the peak
      if(IsPeak(i,j)){
        if(_data_sum[0][i][j]>max_e){
          max_e = _data_sum[0][i][j];
	  max_ix = i;
	  max_iy = j;
	}
      }
    }
  }
  
  return 65*max_ix+max_iy; 
}

int ToHDF5::GetTotHitArea(){
  int tot_area=0;
  for(int i=0;i<NX;i++){
    for(int j=0;j<NY;j++){
      if(_data_sum[0][i][j]>0.000001) tot_area++;
    }
  }

  return tot_area;
}

bool ToHDF5::IsPeak(int ix,int iy){
  if(ix<0 || ix>=NX) return false;
  if(iy<0 || iy>=NY) return false;
  
  for(int i=ix-1;i<=ix+1;i++){
    if(i<0 || i>=NX) continue;
    for(int j=iy-1;j<=iy+1;j++){
      if(j<0 || j>=NY) continue;
      if(i==ix && j==iy) continue;
      if(_data_sum[0][ix][iy]<_data_sum[0][i][j]) return false;
    }
  }
  return true;
}

void ToHDF5::Fill(){
//  std::cout<<"Number of total dataset: "<<_ds_list.size()<<std::endl;
  for(unsigned int i=0;i<_ds_list.size();i++){
    unsigned int rank = _dm_list[i].size()+1;
    hsize_t extend_size[rank];
    hsize_t offset[rank];
    hsize_t fill_dims[rank];
    extend_size[0] = _entries+1;
    offset[0] = _entries;
    fill_dims[0] = 1;
    for(unsigned int j=1;j<rank;j++){
      hsize_t d = _dm_list[i][j-1];
      extend_size[j] = d;
      fill_dims[j] = d;
      offset[j] = 0;
      fill_dims[j] = d;
    }
    
    H5::DataSet* ds = _ds_list[i];
    void* var_pt = _var_pt_list[i];
    
    ds->extend(extend_size);
  
    H5::DataSpace fill_space = ds->getSpace();

    //memory space
    H5::DataSpace mspace(rank,fill_dims);
  
    //select the read position
    fill_space.selectHyperslab(H5S_SELECT_SET,fill_dims,offset);
    if(_dtype_list[i].compare("Float")==0){
      ds->write(var_pt,H5::PredType::NATIVE_FLOAT,mspace,fill_space);
    }

    if(_dtype_list[i].compare("Int")==0){
      ds->write(var_pt,H5::PredType::NATIVE_INT,mspace,fill_space);
    }
  } 
  _entries++;
}


ToHDF5::~ToHDF5(){
  for(unsigned int i=0;i<_ds_list.size();i++){
    if(_ds_list[i]) delete _ds_list[i];
    _ds_list[i]=0;
  }
  _ds_list.clear();

  if(_h5) _h5->close();
  _h5 = 0;
  
}

void ToHDF5::ResetData(){
  for(unsigned int i=0;i<N0;i++){
    for(unsigned int j=0;j<NX;j++){
      for(unsigned int k=0;k<NY;k++){
        _data_sum[i][j][k]=0;
	for(unsigned int m=0;m<NZ;m++){
	  _data[i][j][k][m]=0;
	}
      }
    }
  }
}

/*
void ToHDF5::ToRoot(std::string h5_file,std::string root_file){
  hsize_t num_obj = _h5->getNumObjs();
  std::cout<<"Number of objects: "<<num_obj<<std::endl;

  for(unsigned int i=0;i<num_obj;i++){
    H5std_string obj_name = _h5->getObjnameByIdx(i);
    std::cout<<"Name of object: "<<obj_name<<std::endl;
    H5::DataSet ds = _h5->openDataSet(obj_name);
   // H5::DataType dt = ds.getDataType();
   // H5T_class_t t = dt.getClass();
    H5T_class_t t = ds.getTypeClass();
    std::cout<<"data type: "<<t<<std::endl;
    H5::DataSpace sp = ds.getSpace();
    int Ndims = sp.getSimpleExtentNdims();
    hsize_t r_dims[Ndims];
    sp.getSimpleExtentDims(r_dims);
    std::cout<<"dims: ";
    for(int k=0;k<Ndims;k++){
      std::cout<<r_dims[k]<<" ";
    }
    std::cout<<std::endl;
  }

  TFile* ofile = new TFile("h5toRoot.root","RECREATE");
  TTree* tree = new TTree("h5tree","h5tree");

  for(unsigned int i=0;i<num_obj;i++){
    H5std_string obj_name = _h5->getObjnameByIdx(i);
    std::cout<<"Name of object: "<<obj_name<<std::endl;
    H5::DataSet ds = _h5->openDataSet(obj_name);
    H5T_class_t t = ds.getTypeClass();
    std::cout<<"data type: "<<t<<std::endl;
    H5::DataSpace sp = ds.getSpace();
    int Ndims = sp.getSimpleExtentNdims();
    if(Ndims==1){
      tree->Branch(obj_name,"obj_name",)
    }

    hsize_t r_dims[Ndims];
    sp.getSimpleExtentDims(r_dims);
    std::cout<<"dims: ";
    for(int k=0;k<Ndims;k++){
      std::cout<<r_dims[k]<<" ";
    }
    std::cout<<std::endl;
  }

  return;
}
*/
