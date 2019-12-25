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
typedef std::map<unsigned,float> MSparseMap;

inline MIndex AddIndex(const MIndex& index0,const MIndex& index1){
  //add the index 
  //if index one is (n,m)
  //index two is (s,t)
  //the added one is (n+s,m+t)

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

inline bool CompareIndex(const MIndex& index0,const MIndex& index1){
  for(unsigned int i=0;i<index0.size();i++){
    if(index0[i]<index1[i]) return false;
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
