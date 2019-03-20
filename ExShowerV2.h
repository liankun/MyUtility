#ifndef __EXSHOWERV2_H__
#define __EXSHOWERV2_H__
//include the dominate particle from simulation
//used for simulation
#include "ExShowerV1.h"
#include <vector>
#include <utility>

class TMpcExShower;

class ExShowerV2 : public ExShowerV1{

  public:
     ExShowerV2();
     virtual ~ExShowerV2();
     virtual void ResetExShower();
     virtual void Clear(Option_t *option="");
     virtual void InitShower();
     //first is the track id
     //second is the energy fraction
     //sorted by the energy fraction
     //from higest to lowest
     //in order to get the particle id
     //You need to record primary information
     //for events
     typedef std::pair<int,float> Contributor;

     void SetContributors(TMpcExShower*);
     int GetNContributors(){return _contributors.size();}
     Contributor GetContributor(unsigned int i){
       if(i>=0 && i<_contributors.size())
         return _contributors[i];
       else{
         return Contributor(-1,-1.);
       }
     }
   
   private:
     std::vector<Contributor> _contributors;

   ClassDef(ExShowerV2,1)
};

#endif 
