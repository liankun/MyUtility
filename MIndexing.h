#ifndef __MINDEXING_H__
#define __MINDEXING_H__

/**
 * define some convenient type
 * **/
#include <vector>
#include <map>
#include <iostream>

typedef std::vector<unsigned int> MShape;
typedef std::vector<unsigned int> MIndex;
typedef std::map<unsigned,double> MSparseMap;
typedef std::map<unsigned,double>::const_iterator CIter;

inline MIndex AddIndex(const MIndex& index0,const MIndex& index1){
  //add the index 
  //if index one is (n,m)
  //index two is (s,t)
  //the added one is (n+s,m+t)
  //the size of the index can be different
  //will only add the common dimension index

  unsigned int nsize = index0.size();
  if(nsize<index1.size()) nsize = index1.size();
  
  MIndex add_index = MIndex(nsize,0);
//  if(index0.size()!=index1.size()){
//    std::cout<<"AddIndex: "<<"invalid index !!!"<<std::endl;
//    return add_index;
//  }

  for(unsigned int i=0;i<nsize;i++){
    unsigned int val0 = 0;
    unsigned int val1 = 0;
    if(i<index0.size()) val0 = index0[i];
    if(i<index1.size()) val1 = index1[i];
    add_index[i] = val0+val1;
  }
  return add_index;
}

inline MIndex SubtractIndex(const MIndex& index0,const MIndex& index1){
  //the dimension of the index0 should always equal or larger than the second
  //if the index is invalid, will return a 0 size vector
  //also the value of each index0 >= the second one
  MIndex out_idx;
  if(index0.size()<index1.size()) return out_idx;
  
  for(unsigned int i=0;i<index0.size();i++){
    unsigned int val0 = 0;
    unsigned int val1 = 0;
    if(i<index0.size()) val0 = index0[i];
    if(i<index1.size()) val1 = index1[i];
    if(val0<val1){
      out_idx.clear();
      return out_idx;
    }
    out_idx.push_back(val0-val1);
  }
  return out_idx;
}

inline MIndex DivideIndex(const MIndex& index,const int val){
  //divide a none zero val
  MIndex out_index;
  if(val<=0) return out_index;
  for(unsigned int i=0;i<index.size();i++){
    out_index.push_back(index[i]/val);
  }
  return out_index;
}

inline bool CompareIndex(const MIndex& index0,const MIndex& index1){
  for(unsigned int i=0;i<index0.size();i++){
    if(index0[i]<index1[i]) return false;
  }
  return true;
}

inline bool IndexEqual(const MIndex& index0,const MIndex& index1){
  if(index0.size()!=index1.size()) return false;
  for(unsigned int i=0;i<index0.size();i++){
    if(index0[i]!=index1[i]) return false;
  }

  return true;
}

inline MIndex MultiplyIndex(const MIndex& index,const unsigned int val){
  //each element of the index will be multiply by a same number
  //val
  MIndex mul_index = MIndex(index.size(),0);
  for(unsigned int i=0;i<index.size();i++){
    mul_index[i]=val*index[i];
  }
  return mul_index;
}

inline void PrintIndex(const MIndex& index){
  for(unsigned int i=0;i<index.size();i++){
    std::cout<<index[i]<<" ";
  }
  std::cout<<std::endl;
}

inline void PrintShape(const MShape& shape){
  PrintIndex(shape);
}

#endif /**__MINDEXING_H__**/
