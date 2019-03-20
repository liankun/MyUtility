#ifndef __STFINDER__
#define __STFINDER__

#include <vector>

class ExShower;
class Exogram;
class TSpectrum;
class TH1D;
class TCanvas;
class TSpectrum2;

class STFinder {
  public:
    STFinder();
    STFinder(ExShower*);
    //search two peaks
    //type = 0, all layers
    //type = 1, first 4 layers
    //type = 2, last 4 layers
    bool Search(float sigma=1, float height=0.09,int type = 0);
    bool Search2D(float sigma=1, float height=0.09,int type = 0);
    void Smooth(int times = 1,int type = 0);
    void SetExShower(ExShower* ex_shower);    
    int GetPatID() const {return _patID;}
    float GetXPk(int l) const {if(l>=0&&l<2) return _xpk[l];return -9999;} 
    float GetYPk(int l) const {if(l>=0&&l<2) return _ypk[l];return -9999;}
    float GetXHeight(int l) const {if(l>=0&&l<2) return _xheight[l];return -9999;}
    float GetYHeight(int l) const {if(l>=0&&l<2) return _yheight[l];return -9999;}
    float GetOpenAngle() const {return _open_angle;}
    float GetPi0Mass() const {return _pi0_mass;}
    float GetDPeaks() const {return _dpeaks;}
    int GetXNPks() const {return _x_pks_list.size();}
    int GetYNPks() const {return _y_pks_list.size();}
    float GetRawXPk(unsigned int l) const {if(l<_x_pks_list.size())return _x_pks_list[l];return -9999;}
    float GetRawYPk(unsigned int l) const {if(l<_y_pks_list.size())return _y_pks_list[l];return -9999;}
    
    //clean the hit fill the exogram
    //set the threshold compare to highest q
    void SetCleanThreshold(float val){_threshold = val;}
    //print the exogram
    void Print(int type = 0);
    void SetPrintDir(const char* path){_path = path;}
    void set_layer_scale(){_layer_scale = true;_layer_log_scale=false;}
    void set_layer_log_scale(){_layer_log_scale=true;_layer_scale=false;}

    virtual ~STFinder();
  private:
     TSpectrum* _peakFinder;
     TSpectrum2* _peakFinder2;
     Exogram* _exo;
     float _xpk[2];
     float _ypk[2];
     float _xheight[2];
     float _yheight[2];
     float _pi0_mass;
     float _open_angle;
     int _patID;
     int _first_layer;
     float _shower_e;
     float _vertex;
     float _dpeaks;
     float _shower_x;
     float _shower_y;
     std::vector<float> _x_pks_list;
     std::vector<float> _x_pks_height;
     std::vector<float> _y_pks_list;
     std::vector<float> _y_pks_height;
     int _arm;
     TH1D* _hproject[2];
     //
     TH1D* _hall[2];
     //search first 4 layers
     TH1D* _hfst4[2];
    //search last 4 layers
     TH1D* _hlst4[2];
     const char* _path;
     int _nevent;
     int _type;
     TCanvas* _c;
     float _threshold;
     double _layer_e[2][8];
     double _arm_total_e[2];
     bool _layer_scale;
     bool _layer_log_scale;
     double _layer_scale_factor[8];
};
#endif
