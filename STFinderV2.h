#ifndef __STFINDERV2__
#define __STFINDERV2__

#include <map>
#include <vector>
#include "ExEvent.h"

//a different way to find ST pi0
//first search the peaks in each layer
//and then associate those peaks to
//form a track
//the best two tracks will be considered
//as the sub photons in shower
class STFinderV2{
  public:
    STFinderV2(){
      _hit_map.clear();
      _track_lists.clear();
    }
    
    virtual ~STFinderV2(){
      for(unsigned int i=0;i<_track_lists.size();i++){
        if(_track_lists[i]) delete _track_lists[i];
      }
      _track_lists.clear();
      _hit_map.clear();
    }
    void SetExShower(ExShower* ex_shower);
    void Search(); 
    double GetdPks();
    double GetMass();
    //at least 5 hits in tracks
    void PrintTracks(int N=5);
  private:
    std::map<int,ExHit*> _hit_map;
    double _vertex;
    int _nfired;
    double _shower_e;
    
    typedef std::vector<ExHit*> Track;
    std::vector<Track*> _track_lists;
    int get_index(int arm,int layer,int nx,int ny) const;
    int get_index(ExHit* hit) const;
    int get_nx(ExHit* hit) const;
    int get_ny(ExHit* hit) const;
    //remove large deviation  
    void clean_tracks();
};
#endif
