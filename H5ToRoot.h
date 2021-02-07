#ifndef __H5TOROOT_H__
#define __H5TOROOT_H__

#include <string>
#include <vector>
#include <set>

namespace H5 {
  class H5File;
  class DataSet;
}

class TFile;
class TTree;

class H5ToRoot{
  private:
    H5::H5File* _h5;
    TFile* _r_file;
    TTree* _r_tree;
    std::vector<H5::DataSet*> _data_set_list;
    std::vector<std::string> _table_name_list;
    //only for the internal pointer for writing
    //since we only have two type
    std::vector<int*> _i_list;
    std::vector<float*> _f_list;
    std::vector<bool> _is_array_list;
    unsigned long long int _entries;
    //contain all data set name
    std::set<std::string> _ds_name_set;
    //contain the varaible added
    std::set<std::string> _root_added_leaf_set;

    //settings for each table
    std::vector<std::string> _root_file_leaf;
    
    //Set the root leaf for the branch
    std::string GetLeafSetting(H5::DataSet* ds,int& volume);

  public:
    H5ToRoot(std::string h5_file="Ex.h5");
    virtual ~H5ToRoot();

    void ToRoot(std::string r_file="h5ToRoot.root");
    void AddVar(std::string name);
    void AddAllVars();//add all vars to tree except 2dhits and dp_mpcex_raw_e
    
};

#endif
