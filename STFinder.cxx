#include "STFinder.h"
#include "ExEvent.h"
#include <Exogram.h>
#include <TSpectrum.h>
#include <TSpectrum2.h>
#include <TH2D.h>
#include <TH1D.h>
#include <stdlib.h>
#include <iostream>
#include <MpcExConstants.h>
#include <TCanvas.h>
#include <TDatime.h>
#include <TLine.h>
#include <TCanvas.h>
#include <MpcExMapper.h>

#include <algorithm>

using namespace std;


STFinder::STFinder(){
  _peakFinder = NULL;
  _peakFinder2 = NULL;
  _exo = NULL;
  for(int i=0;i<2;i++){
    _xpk[i] = -9999;
    _ypk[i] = -9999;
    _xheight[i] = -9999;
    _yheight[i] = -9999;
    _hproject[i] = NULL;
    _hall[i] = NULL;
    _hfst4[i] = NULL;
    _hlst4[i] = NULL;
    _arm_total_e[i]=0;
    for(int j=0;j<8;j++){ 
      _layer_e[i][j]=0;
    }
  }
  _dpeaks = -9999;
  _vertex = -9999;
  _shower_e = -9999;
  _pi0_mass = -9999;
  _patID = -9999;
  _open_angle = -9999;
  _first_layer = -9999;
  _arm = -9999;
  _peakFinder = new TSpectrum();
  _peakFinder2 = new TSpectrum2();
  _exo = new Exogram("tempExo", "", 300,-20,20, 300,-20,20, 8,-0.5,7.5, 1);
  _exo->SetDirectory(NULL);
  _x_pks_list.clear();
  _x_pks_height.clear();
  _y_pks_list.clear();
  _y_pks_height.clear();
  _shower_x = -9999;
  _shower_y = -9999;
  _path = NULL;
  _nevent = 0;
  _c = NULL;
  _threshold = 0;
  _layer_scale = false;
  _layer_log_scale = false;
}

STFinder::STFinder(ExShower* exshower){
  STFinder();
  SetExShower(exshower);
}


void STFinder::SetExShower(ExShower* exshower){
  _exo->Reset();
//  cout<<"SetExShower "<<endl;
  for(int i_dim=0;i_dim<2;i_dim++){
    if(_hall[i_dim]){
      delete _hall[i_dim];
//      cout<<"delete hall "<<i_dim<<endl;
      _hall[i_dim] = NULL;
    }

    if(_hfst4[i_dim]){
//      cout<<"delete hfst4 "<<i_dim<<endl;
      delete _hfst4[i_dim];
      _hfst4[i_dim] = NULL;
    }

    if(_hlst4[i_dim]){
//      cout<<"delete hlst4 "<<i_dim<<endl;
      delete _hlst4[i_dim];
      _hlst4[i_dim] = NULL;
    }

  }
 
  unsigned int nhits = exshower->GetNhits();
  MpcExMapper* ex_mapper = MpcExMapper::instance();
  double max_q = -9999;
  for(unsigned int ihit = 0;ihit<nhits;ihit++){
    ExHit* hit_ptr = exshower->GetExHit(ihit);
    double q = hit_ptr->GetE();
    if(max_q < q) max_q = q; 
    int key = hit_ptr->GetKey();
    int layer = ex_mapper->get_layer(key);
    int arm = ex_mapper->get_arm(key);
    _layer_e[arm][layer] += q;
    _arm_total_e[arm] += q;
  }
  
  for(unsigned int ihit=0; ihit<nhits; ihit++){
    ExHit *hit_ptr = exshower->GetExHit(ihit);
    if(hit_ptr->GetE() > max_q*_threshold){
      int key = hit_ptr->GetKey();
      int layer = ex_mapper->get_layer(key);
      int arm = ex_mapper->get_arm(key);
      
      double weight = hit_ptr->GetE();
      if(_layer_e[arm][layer]>0 && _layer_scale) weight = weight/_layer_e[arm][layer];
      if(_layer_e[arm][layer]>0 && _layer_log_scale){ 
        weight = (6+log(hit_ptr->GetE()/_layer_e[arm][layer]))*_layer_e[arm][layer]/_arm_total_e[arm];
	if(weight<0) weight=0;
      }

      _exo->FillEx(hit_ptr->GetKey(),weight);
    }
  }

  _first_layer = exshower->GetFirstLayer();
  _shower_e = exshower->GetTotalE();
  _vertex = exshower->GetVertex();
  _arm = exshower->GetArm();

  if(_arm==1){
    _shower_x = exshower->GetHsx()*(203-_vertex);
    _shower_y = exshower->GetHsy()*(203-_vertex);
  }
  else{
    _shower_x = exshower->GetHsx()*(-203-_vertex);
    _shower_y = exshower->GetHsy()*(-203-_vertex);

  }

  TH2D *h2d_odd = new TH2D("h2d_odd", ";x;y", 300,-20,20, 300,-20,20);
  TH2D *h2d_even = new TH2D("h2d_even", ";x;y", 300,-20,20, 300,-20,20);

  TH2D *h2d_odd_fst4 = new TH2D("h2d_odd_fst4", ";x;y", 300,-20,20, 300,-20,20);
  TH2D *h2d_even_fst4 = new TH2D("h2d_even_fst4", ";x;y", 300,-20,20, 300,-20,20);

  TH2D *h2d_odd_lst4 = new TH2D("h2d_odd_lst4", ";x;y", 300,-20,20, 300,-20,20);
  TH2D *h2d_even_lst4 = new TH2D("h2d_even_lst4", ";x;y", 300,-20,20, 300,-20,20);


   // add even layers together and odd layers together
  for(int ilayer=0; ilayer<MpcExConstants::NLAYERS; ilayer++){
    TH2D *tmp = _exo->GetLayer(ilayer);
    if((ilayer+1)%2!=0){
      h2d_odd->Add(tmp);
      if(ilayer < 4) h2d_odd_fst4->Add(tmp);
      else h2d_odd_lst4->Add(tmp);
    }
    else{
      h2d_even->Add(tmp);
      if(ilayer < 4) h2d_even_fst4->Add(tmp);
      else h2d_even_lst4->Add(tmp);
    }
    delete tmp;
  }

  if(h2d_even->GetEntries()==0 || h2d_odd->GetEntries()==0) {
    delete h2d_even;
    delete h2d_odd;
    delete h2d_odd_fst4;
    delete h2d_even_fst4;
    delete h2d_odd_lst4;
    delete h2d_even_lst4;
    return;
  }

  for(int i_dim=0; i_dim<2; i_dim++){
    _hall[i_dim] = (i_dim==0) ? h2d_odd->ProjectionX("hall_proj_odd_x")
    : h2d_even->ProjectionY("hall_proj_even_y");

    _hfst4[i_dim] = (i_dim==0) ? h2d_odd_fst4->ProjectionX("hfst4_proj_odd_x")
    : h2d_even_fst4->ProjectionY("hfst4_proj_even_y");

    _hlst4[i_dim] = (i_dim==0) ? h2d_odd_lst4->ProjectionX("hlst4_proj_odd_x")
    : h2d_even_lst4->ProjectionY("hlst4_proj_even_y");

    //very important, if a TFile is creadted
    //before this call, then the _hproject will
    //belong to that TFile and the point will delete
    //when the TFile is close, which may crush the 
    //program
    _hall[i_dim]->SetDirectory(NULL);
    _hfst4[i_dim]->SetDirectory(NULL);
    _hlst4[i_dim]->SetDirectory(NULL);
  }
  
  delete h2d_even;
  delete h2d_odd;
  delete h2d_odd_fst4;
  delete h2d_even_fst4;
  delete h2d_odd_lst4;
  delete h2d_even_lst4;

  _nevent++;
}

STFinder::~STFinder(){
   if(_peakFinder){
     delete _peakFinder;
     _peakFinder = NULL;
   }
   if(_peakFinder2){
     delete _peakFinder2;
     _peakFinder2 = NULL;
   }

   if(_exo){
     delete _exo;
     _exo = NULL;
   }

   for(int i=0;i<2;i++){
     if(_hall[i]){
       delete _hall[i];
       _hall[i] = NULL;
     }
     if(_hfst4[i]){
       delete _hfst4[i];
       _hfst4[i] = NULL;
     }
     if(_hlst4[i]){
       delete _hlst4[i];
       _hlst4[i] = NULL;
     }
   }
   _nevent = 0;
   if(_c) delete _c;
}


void STFinder::Smooth(int times,int type){
//  cout<<"Smooth !"<<endl;
  for(int i=0;i<2;i++){
    if(type == 0 && _hall[i]) _hall[i]->Smooth(times);
    else if(type == 1 && _hfst4[i]) _hfst4[i]->Smooth(times);
    else if(type == 2 && _hlst4[i]) _hlst4[i]->Smooth(times);
  }
}

//copy form mMpcExSingleTrackFinder
bool STFinder::Search(float sigma,float height,int type){
//  cout<<"Search "<<endl;
  _x_pks_list.clear();
  _x_pks_height.clear();
  _y_pks_list.clear();
  _y_pks_height.clear();
  for(int i=0;i<2;i++){
    if(type == 0)_hproject[i] = _hall[i];
    else if(type == 1) _hproject[i] = _hfst4[i];
    else if(type == 2) _hproject[i] = _hlst4[i];
    else {
      cout<<"Bad type, only for 0 ,1 and 2"<<endl;
      return false;
    }
  }
  int nPeaksX = 0;
  int nPeaksY = 0;
  
  if(!_hproject[0] || !_hproject[1]){
//    cout<<"No hist avaliable "<<endl;
    _hproject[0] = NULL;
    _hproject[1] = NULL;
    return false;
  }
  
  for(int i_dim=0; i_dim<2; i_dim++){
//     _hproject[i_dim]->Smooth(smooth);
    //do the peak finding in this particular dimension
    int nfound = _peakFinder->Search(_hproject[i_dim],sigma,"goff",height);
    
    //not a valid pi0 if no peaks are found in a dimension: return NULL
    if(nfound == 0){
//      cout<<"No Peak found for "<<i_dim<<endl;      
      _hproject[0] = NULL;
      _hproject[1] = NULL;
      return false;
    }
    
//    if(nfound > 2) nfound = 2;
    for(int i_peak=0; i_peak<nfound; i_peak++){
      double d_peak_pos = _peakFinder->GetPositionX()[i_peak];
      double d_peak_val = _hproject[i_dim]->GetBinContent(_hproject[i_dim]->FindBin(d_peak_pos));
      if(std::abs(d_peak_pos) > 1.e-4 && std::abs(d_peak_pos) < 20.){
	if(i_dim==0){
          _x_pks_list.push_back(d_peak_pos); 
	  _x_pks_height.push_back(d_peak_val);
	  //only record first two peaks
	  if(i_peak<2){
	    _xpk[i_peak] = d_peak_pos;
	    _xheight[i_peak] = d_peak_val;
	    nPeaksX++;
	  }
	}
	else{
          _y_pks_list.push_back(d_peak_pos);
	  _y_pks_height.push_back(d_peak_val);
	  //only record first two peaks
          if(i_peak<2){
	    _ypk[i_peak] = d_peak_pos;
	    _yheight[i_peak] = d_peak_val;
	    nPeaksY++;
	  }
	}
      }//if a valid peak
    }//loop over two peaks
  }//loop over dimension
    
    
  if(nPeaksX == 1 && nPeaksY == 1) _patID = 0; //0
  else if (nPeaksX == 2 && nPeaksY == 1) {_patID = 1; _ypk[1] = _ypk[0];}//1 x2y1
  else if (nPeaksX == 1 && nPeaksY == 2) {_patID = 2; _xpk[1] = _xpk[0];} //2 x1y2
  else if (nPeaksX == 2 && nPeaksY == 2) _patID = 3; //3 x2y2
  else _patID = -1;

  if(_patID <=0){
    _hproject[0] = NULL;
    _hproject[1] = NULL;
//    cout<<"only one peak found "<<endl;
    return false;    
  }
  _dpeaks = sqrt(pow(_xpk[0] -_xpk[1], 2)+pow(_ypk[0] - _ypk[1], 2));
  static const double layerZ[2][4] = {{203.982, 204.636, 205.290, 205.944},
                                      {-203.982, -204.636, -205.290, -205.944}
				      };
  double dz = std::abs(layerZ[_arm][_first_layer]-_vertex);
  double xxx = 0.5*_dpeaks/dz;
  double cosAngle = (1-xxx*xxx)/(1+xxx*xxx);
  _open_angle = 2*std::atan2(0.5*_dpeaks,dz);
  //assumes an asymmetry of alpha = 0.5 so we divide the energy equally between the photons
  _pi0_mass = std::sqrt(0.5*_shower_e*_shower_e*(1-cosAngle));
  
  _hproject[0] = NULL;
  _hproject[1] = NULL;
  
//  cout<<"search finished ! "<<endl;
  return true;
}

bool STFinder::Search2D(float sigma, float height,int type){
   _x_pks_list.clear();
   _x_pks_height.clear();
   _y_pks_list.clear();
   _y_pks_height.clear();

   if(type == 1) _exo->SetAxisRange(0,3,"z");
   else if(type == 2) _exo->SetAxisRange(4,7,"z");
   else _exo->SetAxisRange(0,7,"z");
   TH2D* htemp = (TH2D*)_exo->Project3D("yx");
   htemp->Smooth(1);
   int npeaks = _peakFinder2->Search(htemp,sigma,"goff",height);
   float* xpeaks = _peakFinder2->GetPositionX();
   float* ypeaks = _peakFinder2->GetPositionY();
   for(int p=0;p<npeaks;p++){
     double pos_x = xpeaks[p];
     double pos_y = ypeaks[p];
     if(std::abs(pos_x) < 1.e-4 || std::abs(pos_x) > 20.) continue;
     if(std::abs(pos_y) < 1.e-4 || std::abs(pos_y) > 20.) continue;

     int pos_bin_x = htemp->GetXaxis()->FindBin(pos_x);
     int pos_bin_y = htemp->GetYaxis()->FindBin(pos_y);
     double height = htemp->GetBinContent(pos_bin_x,pos_bin_y);
     
     if(p<2){
       _xpk[p] = pos_x;
       _ypk[p] = pos_y;
       _xheight[p] = height;
       _yheight[p] = height;
     }
     _x_pks_list.push_back(pos_x);
     _y_pks_list.push_back(pos_y);
     _x_pks_height.push_back(height);
     _y_pks_height.push_back(height);
   }
   if(npeaks <= 1){
     _patID = 0;
     return false;
   }
   if(npeaks>1) _patID = 3;
   
   _dpeaks = sqrt(pow(_xpk[0] -_xpk[1], 2)+pow(_ypk[0] - _ypk[1], 2));
   static const double layerZ[2][4] = {{203.982, 204.636, 205.290, 205.944},
                                      {-203.982, -204.636, -205.290, -205.944}
				      };
   double dz = std::abs(layerZ[_arm][_first_layer]-_vertex);
   double xxx = 0.5*_dpeaks/dz;
   double cosAngle = (1-xxx*xxx)/(1+xxx*xxx);
   _open_angle = 2*std::atan2(0.5*_dpeaks,dz);
  //assumes an asymmetry of alpha = 0.5 so we divide the energy equally between the photons
   _pi0_mass = std::sqrt(0.5*_shower_e*_shower_e*(1-cosAngle));
  
//  cout<<"search finished ! "<<endl;
  return true;
}

void STFinder::Print(int type){
  if(!_path){
    cout<<"Path Not Set !!"<<endl;
    return;
  }
  
  if(!_c)_c = new TCanvas("c","c",1200,800);
  if(type == 1) _exo->SetAxisRange(0,3,"z");
  if(type == 2) _exo->SetAxisRange(4,7,"z");
  _exo->SetAxisRange(max(float(-20.),_shower_x-3),min(float(20.),_shower_x+3),"x");
  _exo->SetAxisRange(max(float(-20.),_shower_y-3),min(float(20.),_shower_y+3),"y");
  TH2D* htemp_2d = (TH2D*)_exo->Project3D("yx");
  htemp_2d->Draw("colz");
  
  _c->Update();
  float c_x_min = _c->GetUxmin();
  float c_x_max = _c->GetUxmax();
  float c_y_min = _c->GetUymin();
  float c_y_max = _c->GetUymax();
//  cout<<"c_x_min: "<<c_x_min<<" c_x_max: "<<c_x_max<<endl;
  
  vector<TLine*> line_list;
  for(unsigned int i=0;i<_x_pks_list.size();i++){
    float pos_x = _x_pks_list[i];
//    cout<<"x pos: "<<pos_x<<endl;
    TLine* l = new  TLine(pos_x,c_y_min,pos_x,c_y_max);
    if(i<2 )l->SetLineColor(kRed);
    else l->SetLineColor(kBlack);
    l->SetLineStyle(10);
    l->SetLineWidth(1);
    l->Draw("");
    line_list.push_back(l);
  }
  
  for(unsigned int i=0;i<_y_pks_list.size();i++){
    float pos_y = _y_pks_list[i];
//    cout<<"y pos: "<<pos_y<<endl;
    TLine* l = new TLine(c_x_min,pos_y,c_x_max,pos_y);
    if(i<2)l->SetLineColor(kRed);
    else l->SetLineColor(kBlack);
    l->SetLineStyle(10);
    l->SetLineWidth(1);
    l->Draw("");
    line_list.push_back(l);
  }
  _c->Print(Form("%s/c_%d_%d.gif",_path,_nevent,type));
  _c->Clear();
  for(unsigned int i=0;i<line_list.size();i++){
    delete line_list[i];
  }
  line_list.clear();
  _exo->SetAxisRange(1,-1,"x");
  _exo->SetAxisRange(1,-1,"y");
  _exo->SetAxisRange(1,-1,"z");
  delete htemp_2d;
}
