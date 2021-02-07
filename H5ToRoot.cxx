#include "H5ToRoot.h"
#include <H5Cpp.h>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

H5ToRoot::H5ToRoot(std::string h5_file){
  _h5 = new H5::H5File(h5_file,H5F_ACC_RDONLY);
  if(!_h5){
    std::cout<<"open "<<h5_file<<" failed !"<<std::endl;
    exit(1);
  }
  _entries=0;
  _data_set_list.clear();
  _table_name_list.clear();
  _root_added_leaf_set.clear();
  _i_list.clear();
  _f_list.clear();

  hsize_t num_obj = _h5->getNumObjs();
  
  //get all set name
  for(unsigned int i=0;i<num_obj;i++){
    H5std_string obj_name = _h5->getObjnameByIdx(i);
//    std::cout<<obj_name<<std::endl;
    _ds_name_set.insert(obj_name);
  }
  
  //get the total entries
  if(num_obj>0){
    H5std_string obj_name = _h5->getObjnameByIdx(0);
    H5::DataSet ds = _h5->openDataSet(obj_name);
    H5::DataSpace sp = ds.getSpace();
    int Ndims = sp.getSimpleExtentNdims();
    hsize_t r_dims[Ndims];
    sp.getSimpleExtentDims(r_dims);
    _entries = r_dims[0];
  }

}

void H5ToRoot::AddAllVars(){
  //they are used for classification
  //they will be excluded (they can be add manually)
  std::string name0("2d_hits");
  std::string name1("dp_mpcex_raw_e");
  for(std::set<std::string>::iterator it=_ds_name_set.begin();it!=_ds_name_set.end();++it){
    std::string name = *it;
    if(name.compare(name0)==0) continue;
    if(name.compare(name1)==0) continue;
    AddVar(name);
  } 
}

void H5ToRoot::AddVar(std::string name){
  if(_root_added_leaf_set.find(name)!=_root_added_leaf_set.end()){
    std::cout<<"Data Set "<<name<<" already added ! "<<std::endl;
    return;
  }

  if(_ds_name_set.find(name)==_ds_name_set.end()){
    std::cout<<"DataSet not exist !"<<std::endl;
    return;
  }

  _root_added_leaf_set.insert(name);
    
  H5::DataSet* ds = new H5::DataSet(_h5->openDataSet(name));
  
  
  int volume;
  std::string leaf_str = GetLeafSetting(ds,volume);
  if(volume<0){
    std::cout<<"invalid data set "<<name<<std::endl;
    return;
  }

  _root_file_leaf.push_back(leaf_str);
  
  H5T_class_t t = ds->getTypeClass();
  
  if(t==H5T_INTEGER){
    if(volume==0){
      int* pt = new int();
      _i_list.push_back(pt);
      _f_list.push_back(0);
      _is_array_list.push_back(false);
    }
    else{
      int* pt = new int[volume]();
      _i_list.push_back(pt);
      _f_list.push_back(0);
      _is_array_list.push_back(true);
    }
  }

  if(t==H5T_FLOAT){
    if(volume==0){
      float* pt = new float();
      _f_list.push_back(pt);
      _i_list.push_back(0);
      _is_array_list.push_back(false);
    }
    else{
      float* pt = new float[volume]();
      _f_list.push_back(pt);
      _i_list.push_back(0);
      _is_array_list.push_back(true);
    }
  }


  _data_set_list.push_back(ds);
  _table_name_list.push_back(name);

//  std::cout<<"settings:"<<leaf_str
//           <<" volume:"<<volume
//           <<std::endl;

}

std::string H5ToRoot::GetLeafSetting(H5::DataSet* ds,int& volume){
  std::string leaf_str;
  volume=0;
  if(!ds){
    volume=-1;
    return leaf_str;
  }

  H5T_class_t t = ds->getTypeClass();
  H5::DataSpace sp = ds->getSpace();
  int Ndims = sp.getSimpleExtentNdims();

  std::string obj_name=ds->getObjName();
  obj_name.erase(0,1);
//  std::cout<<obj_name<<std::endl;
     
  hsize_t r_dims[Ndims];
  sp.getSimpleExtentDims(r_dims);
  int t_volume = 1;
  for(int k=1;k<Ndims;k++){
    t_volume*=r_dims[k];
    char i_cstr[500];
    sprintf(i_cstr,"%llu",r_dims[k]);
    std::string t_c_num=i_cstr;
    leaf_str+="["+t_c_num+"]";
  }

  if(Ndims>1) volume = t_volume;

  if(t==H5T_INTEGER){
    if(Ndims==1){
      leaf_str = obj_name+"/I";
    } 
    else{
      leaf_str=obj_name+leaf_str;
      leaf_str+="/I";
    }
  }

  if(t==H5T_FLOAT){
    if(Ndims==1){
      leaf_str = obj_name+"/F";
    }
    else{
      leaf_str=obj_name+leaf_str;
      leaf_str+="/F";
    }
  }
  
  return leaf_str;
}

void H5ToRoot::ToRoot(std::string root_file){
  //set the root branch
  TFile* rfile = new TFile(root_file.c_str(),"RECREATE");
  TTree* h5tree = new TTree("h5tree","h5tree");
  
  if(_entries==0){
    std::cout<<"No Entries !"<<std::endl;
    rfile->Write();
    rfile->Close();
    return;
  }

  //set the branch address
  for(unsigned int i=0;i<_table_name_list.size();i++){
    std::string name = _table_name_list[i];
    std::string r_leaf = _root_file_leaf[i];
    if(_i_list[i]!=0) h5tree->Branch(name.c_str(),_i_list[i],r_leaf.c_str());
    if(_f_list[i]!=0) h5tree->Branch(name.c_str(),_f_list[i],r_leaf.c_str());
  }
  
  
  for(unsigned int i=0;i<_entries;i++){
    //read each line
    for(unsigned int j=0;j<_data_set_list.size();j++){
      H5::DataSet* t_ds = _data_set_list[j];
      H5::DataSpace fill_space = t_ds->getSpace();
      int Ndims = fill_space.getSimpleExtentNdims();
     
      hsize_t r_dims[Ndims];
      fill_space.getSimpleExtentDims(r_dims);
      
      //only read one line
      //this diceide how many rows need to read
      r_dims[0]=1;

      hsize_t offset[Ndims];
      offset[0]=i;
      for(int s=1;s<Ndims;s++) offset[s]=0;
     
      fill_space.selectHyperslab(H5S_SELECT_SET,r_dims,offset);
      H5::DataSpace mspace(Ndims,r_dims);
      if((_i_list[j]==0 && _f_list[j]==0)||(_i_list[j]!=0 && _f_list[j]!=0)){
        std::cout<<"bad news int pointer and float pointer are the same !!!"<<std::endl;
	exit(1);
      }

      if(_i_list[j]!=0) t_ds->read(_i_list[j],H5::PredType::NATIVE_INT,mspace,fill_space);
      if(_f_list[j]!=0) t_ds->read(_f_list[j],H5::PredType::NATIVE_FLOAT,mspace,fill_space);

    }
    h5tree->Fill();
  } 

  rfile->Write();
  rfile->Close();
}


H5ToRoot::~H5ToRoot(){
  for(unsigned int i=0;i<_data_set_list.size();i++){
    if(!_is_array_list[i]){
      if(_i_list[i]){
        delete _i_list[i];
	_i_list[i]=0;
      }
      if(_f_list[i]){
        delete _f_list[i];
	_f_list[i]=0;
      }
    }
    else{
    if(_i_list[i]){
        delete[] _i_list[i];
	_i_list[i]=0;
      }
      if(_f_list[i]){
        delete[] _f_list[i];
	_f_list[i]=0;
      }
    }
    _data_set_list[i]->close(); 
  }
  _h5->close();
}
