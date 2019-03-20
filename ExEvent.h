#ifndef __EXEVENT__
#define __EXEVENT__

#include <TObject.h>
#include <vector>
#include <math.h>

class TMpcExShower;
class TMpcExSingleTrackPi0;

class ExHit : public TObject{
private:
  float _e;
  int _key;
public:
  ExHit(){_e = -9999; _key = -9999;}
  ExHit(int k,float q){_e = q;_key =k;}
  virtual ~ExHit();
  float GetE() {return _e;}
  int GetKey() {return _key;}
  
  //no ";"
  ClassDef(ExHit,1)
};


class ExShower : public TObject{
private:
  int _arm;
  int _first_layer;
  int _fired_layers;
  int _patID;
  int _calib_ok;
  float _centrality;
  float _x_hight[2];
  float _y_hight[2];
  float _x_pk[2];
  float _y_pk[2];
  float _pi0_mass;
  float _vertex;
  float _cor_mpcexE;
  float _raw_mpcexE;
  float _cor_mpcE;
  float _total_e;
  float _hsx;
  float _hsy;
  float _rms_hsx;
  float _rms_hsy;
  float _disp_hsx;
  float _disp_hsy;
  float _layer_hsx[8];
  float _layer_hsy[8];
  float _layer_hsx_rms[8];
  float _layer_hsy_rms[8];
  float _layer_hsx_disp[8];
  float _layer_hsy_disp[8];
  float _layer_e[8];
  int _mpc7x7_ch[7][7];
  float _mpc7x7_e[7][7];
  float _mpc7x7_tof[7][7];
  int _fired_towerN3x3;
  int _fired_towerN5x5;
  float _mpcE3x3;
  float _px;
  float _py;
  float _pz;
  float _phi;
  float _eta;
  float _fit_slope;
  float _fit_intercept;
  float _fit_chi2;
  std::vector<ExHit*> _ex_hits;
  
public:
  ExShower();
  ExShower(TMpcExShower*,TMpcExSingleTrackPi0*);
  ExShower(TMpcExShower*);
  virtual ~ExShower();
  void AddExHit(ExHit*);
  virtual void ResetExShower();
  virtual void Clear(Option_t *option="");
  virtual void InitShower();
  void SetExShower(TMpcExShower* shower);
  void SetSTPi0(TMpcExSingleTrackPi0* pi0,float vtx);
  void SetMpc7x7Ch(int mpc7x7ch[7][7]);
  void SetMpc7x7Tof(float mpc7x7tof[7][7]);
  void SetMpc7x7E(float mpc7x7e[7][7]);
  int GetArm() const {return _arm;}
  int GetFirstLayer() const {return _first_layer;}
  int GetFiredLayers() const {return _fired_layers;}
  int GetPatID() const {return _patID;}
  int GetCalibOK() const {return _calib_ok;}
  float GetXHight(int l) const {if(l<2&&l>=0) return _x_hight[l];return -9999;}
  float GetYHight(int l) const {if(l<2&&l>=0) return _y_hight[l];return -9999;}
  float GetXPk(int l) const {if(l<2&&l>=0) return _x_pk[l];return -9999;}
  float GetYPk(int l) const {if(l<2&&l>=0) return _y_pk[l];return -9999;}
  float GetdPks() const {if(_patID > 0) return sqrt(pow(_x_pk[0]-_x_pk[1],2)+pow(_y_pk[0]-_y_pk[1],2));return -9999;}
  float GetPi0Mass() const {return _pi0_mass;}
  float GetVertex() const {return _vertex;}
  float GetCorMpcexE() const {return _cor_mpcexE;}
  float GetRawMpcexE() const {return _raw_mpcexE;}
  float GetCorMpcE() const {return _cor_mpcE;}
  float GetTotalE() const {return _total_e;}
  float GetHsx() const {return _hsx;}
  float GetHsy() const {return _hsy;}
  float GetRmsHsx() const {return _rms_hsx;}
  float GetRmsHsy() const {return _rms_hsy;}
  float GetDispHsx() const {return _disp_hsx;}
  float GetDispHsy() const {return _disp_hsy;}
  float GetLayerHsx(int l) const {if(l<8&&l>=0) return _layer_hsx[l]; return -9999;}
  float GetLayerHsy(int l) const {if(l<8&&l>=0) return _layer_hsy[l]; return -9999;}
  float GetLayerHsxRms(int l) const {if(l<8&&l>=0) return _layer_hsx_rms[l]; return -9999;}
  float GetLayerHsyRms(int l) const {if(l<8&&l>=0) return _layer_hsy_rms[l]; return -9999;}
  float GetLayerHsxDisp(int l) const {if(l<8&&l>=0) return _layer_hsx_disp[l];return -9999;}
  float GetLayerHsyDisp(int l) const {if(l<8&&l>=0) return _layer_hsy_disp[l];return -9999;}
  float GetLayerE(int l) const {if(l<8&&l>=0) return _layer_e[l]; return -9999;}
  int GetMpc7x7Ch(int l,int k) const {if(l<7&&l>=0&&k<7&&k>=0) return _mpc7x7_ch[l][k];return -9999;}
  float GetMpc7x7E(int l,int k) const {if(l<7&&l>=0&&k<7&&k>=0) return _mpc7x7_e[l][k];return -9999;}
  float GetMpc7x7Tof(int l,int k) const {if(l<7&&l>=0&&k<7&&k>=0) return _mpc7x7_tof[l][k];return -9999;}
  int GetFiredTowerN3x3() const {return _fired_towerN3x3;}
  int GetFiredTowerN5x5() const {return _fired_towerN5x5;}
  float GetMpcE3x3() const {return _mpcE3x3;}
  float GetPx() const {return _px;}
  float GetPy() const {return _py;}
  float GetPz() const {return _pz;}
  float GetPhi() const {return _phi;}
  float GetEta() const {return _eta;}
  float GetFitSlope() const {return _fit_slope;}
  float GetFitIntercept() const {return _fit_intercept;}
  float GetFitChi2() const {return _fit_chi2;} 
  int GetNhits() const {return _ex_hits.size();}
  ExHit* GetExHit(unsigned int l) {if(l < _ex_hits.size())return _ex_hits[l]; else return 0;}
  

  // no ";"
  ClassDef(ExShower,1)
};



class ExEventHeader {
private:
  float _centrality;
  float _vertex;
  int _run_number;
  int _trigger;
  int _nhits_north;
  int _nhits_south;
  int _nshowers;
public:
  ExEventHeader():_centrality(-9999),_vertex(-9999),_run_number(-9999),_trigger(-9999),_nhits_north(-9999),_nhits_south(-9999),_nshowers(-9999) {}
  virtual ~ExEventHeader(){}
  float GetCentrality() {return _centrality;}
  float GetVertex() {return _vertex;}
  int GetRunNumber() {return _run_number;}
  int GetTrigger() {return _trigger;}
  int GetNorthNhits() {return _nhits_north;}
  int GetSouthNhits() {return _nhits_south;}
  int GetNShowers() {return _nshowers;}
  
  void SetCentrality(float val) {_centrality = val;}
  void SetVertex(float val) {_vertex = val;}
  void SetRunNumber(int val) {_run_number = val;}
  void SetTrigger(int val) {_trigger = val;}
  void SetNhits(int val_north,int val_south) {_nhits_north = val_north;_nhits_south = val_south;}
  void SetNShowers(int val) {_nshowers = val;}

  ClassDef(ExEventHeader,1)
};

class ExEvent : public TObject{
private:
  float _centrality;
  float _vertex;
  int _run_number;
  int _nhits_north;
  int _nhits_south;
  int _trigger;
  std::vector<ExShower*> _ex_showers;
public:
  ExEvent();
  ExEvent(ExEventHeader header);
  virtual ~ExEvent();
  virtual void Clear(Option_t *option="");
  virtual void Reset();
  void AddExShower(ExShower*);
  float GetCentrality() const {return _centrality;}
  float GetVertex() const {return _vertex;}
  int GetNShowers() const {return _ex_showers.size();}
  ExShower* GetExShower(unsigned int l) {if(l<_ex_showers.size()) return _ex_showers[l];else return 0;}
  int GetTrigger(const char* trgName = "BBCLL1(>0 tubes)");
  int GetRunNumber() const {return _run_number;}
  int GetNorthNhits() const {return _nhits_north;}
  int GetSouthNhits() const {return _nhits_south;}
  void SetCentrality(float cent) {_centrality = cent;}
  void SetVertex(float vtx) {_vertex = vtx;}
  void SetRunNumber(int runNumber){_run_number = runNumber;}
  void SetTrigger(int trig){_trigger = trig;}
  void SetNhits(int n_north,int n_south) {_nhits_north = n_north;_nhits_south = n_south;}

//no ";"
  ClassDef(ExEvent,1)
};

#endif
