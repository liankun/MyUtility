#include "ExMiniClusters.h"
#include "MiniClusterV1.h"
#include <TMpcExShower.h>
#include <TMpcExHitContainer.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <Exogram.h>
#include <MpcExMapper.h>
#include <algorithm>
#include <TEllipse.h>
#include <TLine.h>
#include "ExGeomToKey.h"
#include "ExEvent.h"

using namespace std;

void Square::Print(){
  cout<<"square: "
      <<" x: "<<_x
      <<" y: "<<_y
      <<" ix: "<<_ix
      <<" iy: "<<_iy
      <<" E: "<<GetE()
      <<endl;
}

Square* Square::Clone(){
  Square* sq = new Square();
  sq->SetX(_x);
  sq->SetY(_y);
  sq->SetGridX(_ix);
  sq->SetGridY(_iy);
  for(int i=0;i<8;i++){
    sq->SetLayerE(i,_layer_e[i]);
  }

  return sq;
}

ExMiniClusters::ExMiniClusters(){
  _debug = false;
  _th_rms = 2.2;
  _th_rms_asy = 0.5;
  _th_pk_mean_dist = 0.5;
  _max_iterate = 100;
  _sq_cuts = false;
  _mini_cluster_list.clear();
  _delete_sq_list.clear();
}

ExMiniClusters::ExMiniClusters(ExShower* ex_shower){
  _mini_cluster_list.clear();
  _delete_sq_list.clear();
  _debug = false;
  _th_rms = 2.2;
  _th_rms_asy = 0.5;
  _th_pk_mean_dist = 0.5;
  _max_iterate = 100;
  _sq_cuts = false;
  ConstructMiniClusters(ex_shower);
}

ExMiniClusters::ExMiniClusters(TMpcExShower* shower,TMpcExHitContainer* hits){ 
  _mini_cluster_list.clear();
  _delete_sq_list.clear();
  _debug = false;
  _th_rms = 2.2;
  _th_rms_asy = 0.5;
  _th_pk_mean_dist = 0.5;
  _max_iterate = 100;
  _sq_cuts = false;
  ConstructMiniClusters(shower,hits);
}


ExMiniClusters::~ExMiniClusters(){
  Reset();
}

void ExMiniClusters::Reset(){
  for(unsigned int i=0;i<_mini_cluster_list.size();i++){
    if(_mini_cluster_list[i]) delete _mini_cluster_list[i];
    _mini_cluster_list[i] = NULL;
  }
  _mini_cluster_list.clear();

  for(unsigned int i=0;i<_delete_sq_list.size();i++){
    if(_delete_sq_list[i]) delete _delete_sq_list[i];
    _delete_sq_list[i] = NULL;
  }
  _delete_sq_list.clear();
}

unsigned int ExMiniClusters::GetNMiniCluster(){
  return _mini_cluster_list.size();
}

MiniCluster* ExMiniClusters::GetMiniCluster(unsigned int i){
  if(i>=_mini_cluster_list.size()) return NULL;
  return _mini_cluster_list[i];
}

//make compare function for square

bool square_cmp(Square* sq0,Square* sq1){
  double e0 = sq0->GetE();
  double e1 = sq1->GetE();
  return e0 > e1; 
}


void ExMiniClusters::ConstructMiniClusters(ExShower* ex_shower){
  if(!ex_shower) return;
  Reset();

  //create square
  //we will divide 40cmx40cm into 214x214 square
  const int nsquare = 214;
  double dspace = 40./nsquare;
  double space_min = -20;
//  double space_max =  20;

  //make a square array
  //this is map, we can use the index
  //to fast access the Square;
  Square* sq_array[nsquare][nsquare];
  memset(sq_array,0,nsquare*nsquare*sizeof(Square*));
  //Square list for delete
  std::vector<Square*> sq_list;

  ExGeomToKey* g2k = ExGeomToKey::instance();
  MpcExMapper* ex_mapper = MpcExMapper::instance();
  
  //for fast look up
  ExHit* hit_array[49152];
  memset(hit_array,0,49152*sizeof(ExHit*));
  //make map for look up

  for(int i=0;i<ex_shower->GetNhits();i++){
    ExHit* hit = ex_shower->GetExHit(i);
    int key = hit->GetKey();
    hit_array[key] = hit;
  }

  //fill each square
  for(int i=0;i<ex_shower->GetNhits();i++){
    ExHit* hit = ex_shower->GetExHit(i);
    int key = hit->GetKey();
    int layer = ex_mapper->get_layer(key);
    int arm = ex_mapper->get_arm(key);
    double x = ex_mapper->get_x(key);
    double y = ex_mapper->get_y(key);
    double q = hit->GetE();

    //search overlap region
    //the ratio of the lenght /width is 8
    //we search following layer first and then
    //previous layer
    for(int s=0;s<2;s++){
      int t_layer = layer-2*s+1;
      if(t_layer>7 || t_layer<0) continue;
      std::vector<double> pos_list;
      std::vector<double> q_list;
      std::set<int> used_set;
      double q_sum=0;
     
      for(int t=0;t<8;t++){
        double t_x = x; 
	double t_y = y+0.1876*t-0.65625;
	if(layer%2==1){
	  t_x = x+0.1876*t-0.65625;
	  t_y = y;
	}

        int t_key = g2k->get_key(arm,t_layer,t_x,t_y);
	if(t_key<0) continue;
	if(!hit_array[t_key]) continue;
	
	if(used_set.find(t_key)!=used_set.end()) continue;
        ExHit* t_hit = hit_array[t_key];
	double t_q = t_hit->GetE();
        t_x = ex_mapper->get_x(t_key);
        t_y = ex_mapper->get_y(t_key);
	double pos = t_y;
	if(layer%2==1) pos = t_x;
	else pos = t_y;
	pos_list.push_back(pos);
	q_list.push_back(t_q);
	q_sum += t_q;
      }
      
      if(q_sum<=0) continue;

      for(unsigned int k=0;k<pos_list.size();k++){
        
	double t_sq_x = x;
	double t_sq_y = pos_list[k];

	if(layer%2==1){
	  t_sq_x = pos_list[k];
	  t_sq_y = y;
	}
	
	int gix = (t_sq_x-space_min)/dspace;
	int giy = (t_sq_y-space_min)/dspace;

	if(sq_array[gix][giy]){
	  Square* sq = sq_array[gix][giy];
	  double cur_layer_e = sq->GetLayerE(t_layer);
          sq->SetLayerE(t_layer,cur_layer_e+q*q_list[k]/q_sum);
	}
	else{
	  Square* sq = new Square();
	  sq->SetX(t_sq_x);
	  sq->SetY(t_sq_y);
	  sq->SetGridX(gix);
	  sq->SetGridY(giy);
	  sq->SetLayerE(t_layer,q*q_list[k]/q_sum);
	  sq_array[gix][giy] = sq;
	  sq_list.push_back(sq);
	}
      }
      break;
    }
  }
  
  if(_debug) print_square_2d(sq_list,ex_shower);
  
  
  //make miniclusters
  //start from largest energy
  //if the distance is less then 1 rms 
  //then include the square

  //search all peaks
  //if one square is larger than its surrondings
  //these peak will used as peak
  
  std::sort(sq_list.begin(),sq_list.end(),square_cmp);

  //check if we get the right value
  if(_debug){
    for(unsigned int i=0;i<sq_list.size();i++){
      cout<<"ordered square:  "<<i<<"  "
          <<"E "<<sq_list[i]->GetE()<<endl;
    }
  }

  
  //loop pk first , then non pk list
  //0 is pk and 1 is non peak
  std::vector<Square*> sq_pk_non_pk_list[2];
  for(unsigned int i=0;i<sq_list.size();i++){
    int sq_ix = sq_list[i]->GetGridX();
    int sq_iy = sq_list[i]->GetGridY();
    double sq_e = sq_list[i]->GetE();
    
    //ignore this square
    if(_sq_cuts && !IsGoodSq(sq_list[i])){
      _delete_sq_list.push_back(sq_list[i]);
      sq_array[sq_ix][sq_iy] = 0;
      continue;
    }


    bool is_peak=true;
    for(int idx=-1;idx<=1;idx++){
      for(int idy=-1;idy<=1;idy++){
        
	if(idx==0 && idy==0) continue;
         //rule out index which are out of range
        int tmp_sq_ix = sq_ix+idx;
	int tmp_sq_iy = sq_iy+idy;
	if(tmp_sq_ix<0 || tmp_sq_ix>=nsquare) continue;
	if(tmp_sq_iy<0 || tmp_sq_iy>=nsquare) continue;

	//not exists
	if(!sq_array[tmp_sq_ix][tmp_sq_iy]) continue;
	double tmp_sq_e = sq_array[tmp_sq_ix][tmp_sq_iy]->GetE();
	//not peak
	if(tmp_sq_e>sq_e){ 
          is_peak = false;
	  break;
	}
      }//idy
      if(!is_peak) break;
    }//idx
    
    if(is_peak) sq_pk_non_pk_list[0].push_back(sq_list[i]);
    else sq_pk_non_pk_list[1].push_back(sq_list[i]);
  }//i

  if(_debug){
    for(unsigned i_d=0;i_d<sq_pk_non_pk_list[0].size();i_d++){
      cout<<"pk list: "<<i_d<<" E "<<sq_pk_non_pk_list[0][i_d]->GetE()<<endl;
    }
  }

  //use these peak as seed to clusters
  //then non the left none peak values;
  bool used_square[nsquare][nsquare];
  //1 is used and 0 is not used
  memset(used_square,0,nsquare*nsquare*sizeof(bool));

  for(int i_is_pk=0;i_is_pk<2;i_is_pk++){
    for(unsigned int i=0;i<sq_pk_non_pk_list[i_is_pk].size();i++){
      int sq_ix = sq_pk_non_pk_list[i_is_pk][i]->GetGridX();
      int sq_iy = sq_pk_non_pk_list[i_is_pk][i]->GetGridY();
      double sq_x = sq_pk_non_pk_list[i_is_pk][i]->GetX();
      double sq_y = sq_pk_non_pk_list[i_is_pk][i]->GetY();
      
      if(_debug){
//        cout<<"pk non pk set: "<<i_is_pk<<endl;
//	cout<<"sq i "<<i<<" E "<<sq_pk_non_pk_list[i_is_pk][i]->GetE()<<endl;
      }

      if(used_square[sq_ix][sq_iy]){
 //       if(_debug) cout<<"sq E "<<sq_pk_non_pk_list[i_is_pk][i]->GetE()<<" used "<<endl;
	continue;
      }
      
      //used_square[sq_ix][sq_iy] = true;
      //use this as seed, first,3x3 array,
      double sum_x = 0;
      double sum_x2 = 0;
      double sum_y = 0;
      double sum_y2 = 0;
      double norm = 0;
      
      //define the outline of the mxn box
      int left = 0;
      int right = 0;
      int up = 0;
      int down = 0;

      if(_debug){
        cout<<"create mini cluster "<<endl;
        sq_pk_non_pk_list[i_is_pk][i]->Print();
      } 
      
      
      MiniClusterV1* mini_cluster = new MiniClusterV1();
      mini_cluster->SetShowerE(ex_shower->GetTotalE());
      mini_cluster->SetVertex(ex_shower->GetVertex());
      double zdist = 205 - ex_shower->GetVertex();
      if(ex_shower->GetArm()==0) zdist = -205 - ex_shower->GetVertex();

      mini_cluster->SetShowerX(ex_shower->GetHsx()*zdist);
      mini_cluster->SetShowerY(ex_shower->GetHsy()*zdist);
      
      _mini_cluster_list.push_back((MiniCluster*)mini_cluster);
      
      if(i_is_pk==0) mini_cluster->SetIsSeedPk(true);
      else mini_cluster->SetIsSeedPk(false);

      mini_cluster->SetPkX(sq_pk_non_pk_list[i_is_pk][i]->GetX());
      mini_cluster->SetPkY(sq_pk_non_pk_list[i_is_pk][i]->GetY());
     
      mini_cluster->SetPkGridX(sq_pk_non_pk_list[i_is_pk][i]->GetGridX());
      mini_cluster->SetPkGridY(sq_pk_non_pk_list[i_is_pk][i]->GetGridY());


      //set layer E
      for(int ilayer=0;ilayer<8;ilayer++){ 
        mini_cluster->SetLayerPkE(ilayer,sq_pk_non_pk_list[i_is_pk][i]->GetLayerE(ilayer));
      }

      int niter = 0;
      while(niter<_max_iterate){ 
        //try nxm array square when enlarge 
	//each time we only iter out range
	//for example
	//1 2 3
	//4 5 6
	//7 8 9
	//we iter all except 5
	//the algo is performed as following
	//when iud (iter y) is located on bottom or top
	//we set the iter step to 1
	//if iud is betwen center (like 4 5 6)
	//set the step to 2, then we will skip 5
        
	//make sure we update for each iteration
        bool is_update = false;
	double sq_nxn_e = 0;

	for(int iud=sq_iy-down;iud<=sq_iy+up;iud++){
	  if(iud>=nsquare || iud<0) continue;
	  //iterate step
	  int it_step=1;
	  if(iud>sq_iy-down && iud<sq_iy+up) it_step = left+right;
	  //for begining when left, right are the same
	  //set it_step to 1 to avoid infinit loop
	  if(it_step==0) it_step=1;
	  for(int ilr=sq_ix-left;ilr<=sq_ix+right;ilr+=it_step){
	    if(ilr>=nsquare || ilr<0) continue;
            if(used_square[ilr][iud]) continue;
	    //if the square exist or not
            if(sq_array[ilr][iud]){
              used_square[ilr][iud]=true;
              double tmp_x = sq_array[ilr][iud]->GetX();
              double tmp_y = sq_array[ilr][iud]->GetY();
              double tmp_e = sq_array[ilr][iud]->GetE();
	      sq_nxn_e+= tmp_e;
              if(_debug) cout<<"square E: "<<tmp_e<<endl;
              sum_x += tmp_e*tmp_x;
              sum_x2 += tmp_e*tmp_x*tmp_x;
              sum_y += tmp_e*tmp_y;
              sum_y2 += tmp_e*tmp_y*tmp_y;
              norm += tmp_e;
              mini_cluster->InsertSq(sq_array[ilr][iud]);
              for(int ilayer=0;ilayer<8;ilayer++){ 
                double add_e = sq_array[ilr][iud]->GetLayerE(ilayer);
                double cur_e = mini_cluster->GetLayerE(ilayer);
                mini_cluster->SetLayerE(ilayer,add_e+cur_e);
              }//ilayer
              is_update = true;
            }//if
	  }//ilr
	}//iud

        //isolated
	if(!is_update){
	  if(_debug) cout<<"no new update !"<<endl;
	  break;
	}
        
        if(sq_nxn_e>0) mini_cluster->InsertSqENxN(sq_nxn_e);

	if(norm==0){
	  cout<<WHERE<<" norm is zero should not happen"<<endl;
	  return;
	}

	double tmp_mean_x = sum_x/norm;
        double tmp_mean_y = sum_y/norm;
        double tmp_mean_x2 = sum_x2/norm;
        double tmp_mean_y2 = sum_y2/norm;

        double tmp_rms_x = sqrt(tmp_mean_x2-tmp_mean_x*tmp_mean_x+1.0e-10);
	if(tmp_rms_x<=1.0e-4) tmp_rms_x=0.2/sqrt(12.);
        if(_debug) cout<<"tmp_rms_x "<<tmp_rms_x<<endl;

        double tmp_rms_y = sqrt(tmp_mean_y2-tmp_mean_y*tmp_mean_y+1.0e-10);
	if(tmp_rms_y<=1.0e-4) tmp_rms_y=0.2/sqrt(12.);
	if(_debug) cout<<"tmp_rms_y "<<tmp_rms_y<<endl;

        double tmp_rms_r = sqrt(tmp_rms_x*tmp_rms_x+tmp_rms_y*tmp_rms_y+1.0e-10);
        
	if(niter==0){
	//first iteration
	//enlarge all
	  left+=1;
	  right+=1;
	  up+=1;
	  down+=1;
	}
	else{
	  //the distance to mean center
	  double dleft = (left+1)*dspace+(tmp_mean_x-sq_x);
	  if(dleft<_th_rms*(tmp_rms_r)) left++;
	  double dright = (right+1)*dspace+(sq_x-tmp_mean_x);
	  if(dright<_th_rms*(tmp_rms_r)) right++;
	  double dup = (up+1)*dspace+(sq_y-tmp_mean_y);
	  if(dup<_th_rms*(tmp_rms_r)) up++;
	  double ddown = (down+1)*dspace+(tmp_mean_y-sq_y);
	  if(ddown<_th_rms*(tmp_rms_r)) ddown++;
	}

	if(tmp_rms_x+tmp_rms_y>0.00001){
	  
	  double tmp_rms_asy = fabs(tmp_rms_x-tmp_rms_y)/(tmp_rms_x+tmp_rms_y);
	  if(tmp_rms_asy>0.5){
	    if(_debug) cout<<"large rms asymmetry !"<<endl;
	    break;
	  }

	  //pk distance to mean x and y
	  double pk_m_dx = fabs(sq_x - tmp_mean_x);
	  double pk_m_dy = fabs(sq_y - tmp_mean_y);
	  //double pk_m_dr = sqrt(pow(pk_m_dx,2)+pow(pk_m_dy,2));
	  if(pk_m_dx>_th_pk_mean_dist*tmp_rms_x) break;
	  if(pk_m_dy>_th_pk_mean_dist*tmp_rms_y) break;
	  //if(pk_m_dr>_th_pk_mean_dist*tmp_rms_r) break;

	}
        niter++;
      }//while
      
      if(_debug) cout<<"iterator number: "<<niter<<endl;
      
      double mean_x = sum_x/norm;
      double mean_y = sum_y/norm;
      double mean_x2 = sum_x2/norm;
      double mean_y2 = sum_y2/norm;

      double rms_x = sqrt(mean_x2-mean_x*mean_x+1.0e-10);
      if(rms_x<=1.0e-4) rms_x=0;
      double rms_y = sqrt(mean_y2-mean_y*mean_y+1.0e-10);
      if(rms_y<=1.0e-4) rms_y=0;

      mini_cluster->SetX(mean_x);
      mini_cluster->SetY(mean_y);
      mini_cluster->SetRMSX(rms_x);
      mini_cluster->SetRMSY(rms_y);
      mini_cluster->SetRadius(sqrt(pow((left+right)/2.,2)/2.+pow((up+down)/2.,2)/2.)*dspace+0.5*dspace);
    }//i
  }


  //remove the pointer for the debug test;
//  for(unsigned int i=0;i<sq_list.size();i++){
//    if(sq_list[i]){ 
//      delete sq_list[i];
//      sq_list[i] = NULL;
//    }
//  }
//  sq_list.clear();


  return;
}

void ExMiniClusters::ConstructMiniClusters(TMpcExShower* shower,TMpcExHitContainer* hits){
  //for now nothing to do
  return;
}

void ExMiniClusters::print_square_2d(vector<Square*> sq_list,ExShower* ex_shower){
  static int ct = 0;
  if(!ex_shower || sq_list.size()==0) return;
  
  std::vector<TH1*> delete_list;   
 
  TH2D* hsquare_2d = new TH2D(Form("hsquare_2d_%d",ct),"Square 2D",214,-20,20,214,-20,20);
  hsquare_2d->GetXaxis()->SetTitle("X/cm");
  hsquare_2d->GetYaxis()->SetTitle("Y/cm");
  delete_list.push_back(hsquare_2d);

  Exogram* hexo = new Exogram(Form("hexo_%d",ct),"Exogram",600,-20,20,600,-20,20,8,-0.5,7.5,1);
  delete_list.push_back(hexo);

  for(unsigned int i=0;i<sq_list.size();i++){
    hsquare_2d->Fill(sq_list[i]->GetX(),
                    sq_list[i]->GetY(),
		    sq_list[i]->GetE());
  }
  
  int nhits = ex_shower->GetNhits(); 
  for(int i=0;i<nhits;i++){
    ExHit* hit = ex_shower->GetExHit(i);
    int key = hit->GetKey();
    double q = hit->GetE();
    hexo->FillEx(key,q);
  }

  TH2D* htemp_2d = (TH2D*)(hexo->Project3D("yx"));
  delete_list.push_back(htemp_2d);

  double mean_x = htemp_2d->GetMean(1);
  double mean_y = htemp_2d->GetMean(2);
  double rms_x = htemp_2d->GetRMS(1);
  double rms_y = htemp_2d->GetRMS(2);

  htemp_2d->SetAxisRange(mean_x-5*rms_x,mean_x+5*rms_x,"x");
  htemp_2d->SetAxisRange(mean_y-5*rms_y,mean_y+5*rms_y,"y");

  hsquare_2d->SetAxisRange(mean_x-5*rms_x,mean_x+5*rms_x,"x");
  hsquare_2d->SetAxisRange(mean_y-5*rms_y,mean_y+5*rms_y,"y");

  TCanvas* c = new TCanvas(Form("c_%d",ct),"c",1600,800);
  c->Divide(3,2);
  c->cd(1);
  htemp_2d->Draw("colz");
  c->cd(4);
  hsquare_2d->Draw("colz");
  c->cd(2);
  TH1D* htemp_1d = htemp_2d->ProjectionX();
  delete_list.push_back(htemp_1d);
  htemp_1d->Draw();
  c->cd(5);
  htemp_1d = hsquare_2d->ProjectionX();
  htemp_1d->Draw();
  c->cd(3);
  htemp_1d = htemp_2d->ProjectionY();
  delete_list.push_back(htemp_1d);
  htemp_1d->Draw();
  c->cd(6);
  htemp_1d = hsquare_2d->ProjectionY();
  delete_list.push_back(htemp_1d);
  htemp_1d->Draw();
  
  TFile* ofile = NULL;
  if(ct>0) ofile = new TFile("Debug_ExMiniClusters_shower_square_2d.root","UPDATE");
  else{
    gSystem->Exec("rm -f Debug_ExMiniClusters_shower_square_2d.root");
    ofile = new TFile("Debug_ExMiniClusters_shower_square_2d.root","RECREATE"); 
  }

  c->Write();
  ofile->Close();

  for(unsigned int i=0;i<delete_list.size();i++){
    if(delete_list[i]) delete delete_list[i];
    delete_list[i] = NULL;
  }

  delete_list.clear();
  ct++;
}

void ExMiniClusters::VisualMiniClusters(ExShower* ex_shower,const char* dataset){
  if(_mini_cluster_list.size()<=0) return;
  static int ct = 0;

  vector<TObject*> _delete_list;

  double reco_e = -9999;
  double mpc_e3x3 = -9999;
  double mpcex_e = -9999;
  double pi0_mass = -9999;
  if(ex_shower){
    reco_e= ex_shower->GetTotalE();
    mpc_e3x3 = ex_shower->GetMpcE3x3();
    mpcex_e = ex_shower->GetCorMpcexE();
    pi0_mass = ex_shower->GetPi0Mass();
  }

  TH2D* hsquare_2d = new TH2D(Form("hsquare_2d_%d",ct),
                             Form("Reco E %.3f MpcE3x3: %.3f MpcExE: %.3f Pi0 Mass:%.3f",
			         reco_e,mpc_e3x3,mpcex_e,pi0_mass),
			     214,-20,20,214,-20,20);
  hsquare_2d->GetXaxis()->SetTitle("X/cm");
  hsquare_2d->GetYaxis()->SetTitle("Y/cm");
  
  _delete_list.push_back(hsquare_2d);
 
  TH2D* hsquare_2d_w_e = new TH2D(Form("hsquare_2d_%d_w_e",ct),"Square 2D",214,-20,20,214,-20,20);
  hsquare_2d_w_e->GetXaxis()->SetTitle("X/cm");
  hsquare_2d_w_e->GetYaxis()->SetTitle("Y/cm");

  _delete_list.push_back(hsquare_2d_w_e);

  if(_debug) cout<<"Number of MiniCluster: "<<_mini_cluster_list.size()<<endl;
  for(unsigned int i=0;i<_mini_cluster_list.size();i++){
    int Nsq = _mini_cluster_list[i]->GetNSq();
//    if(_debug) cout<<"Number of Squares: "<<Nsq<<endl;
    for(int j=0;j<Nsq;j++){
      Square* sq = _mini_cluster_list[i]->GetSq(j);
      hsquare_2d->Fill(sq->GetX(),
                       sq->GetY(),
		       sq->GetE());
      hsquare_2d_w_e->Fill(sq->GetX(),
                       sq->GetY(),
		       _mini_cluster_list[i]->GetExE());
    }
    hsquare_2d_w_e->Fill(_mini_cluster_list[i]->GetPkX(),
                         _mini_cluster_list[i]->GetPkY(),
		         _mini_cluster_list[i]->GetExE());

  }

  double mean_x = hsquare_2d->GetMean(1);
  double mean_y = hsquare_2d->GetMean(2);
  double rms_x = hsquare_2d->GetRMS(1);
  double rms_y = hsquare_2d->GetRMS(2);

  hsquare_2d->SetAxisRange(mean_x-6*rms_x,mean_x+6*rms_x,"x");
  hsquare_2d->SetAxisRange(mean_y-6*rms_y,mean_y+6*rms_y,"y");

  hsquare_2d_w_e->SetAxisRange(mean_x-6*rms_x,mean_x+6*rms_x,"x");
  hsquare_2d_w_e->SetAxisRange(mean_y-6*rms_y,mean_y+6*rms_y,"y");

  TCanvas* c = new TCanvas(Form("c_visual_mini_clusters_%d",ct),"c",1600,800);
  c->Divide(2,1);
  _delete_list.push_back(c);
  
  c->cd(1);
  hsquare_2d->Draw("colz");

  for(unsigned int i=0;i<_mini_cluster_list.size();i++){
    double pk_x = _mini_cluster_list[i]->GetX();
    double pk_y = _mini_cluster_list[i]->GetY();
    double r = _mini_cluster_list[i]->GetRadius();
    double r_rms = _mini_cluster_list[i]->GetRMS();
    
    TLine* t_l = new TLine(pk_x,std::max(-20.,pk_y-r/2.),pk_x,std::min(pk_y+r/2.,20.));
    _delete_list.push_back(t_l);
    t_l->SetLineWidth(2);
    t_l->Draw("same");

    t_l = new TLine(std::max(-20.,pk_x-r/2.),pk_y,std::min(20.,pk_x+r/2.),pk_y);
    _delete_list.push_back(t_l);
    t_l->SetLineWidth(2);
    t_l->Draw("same");

    TEllipse* elp = new TEllipse(pk_x,pk_y,r,r);
    elp->SetFillStyle(0);
    if(_mini_cluster_list[i]->IsSeedPk()){
      elp->SetLineColor(kRed);
    }

    _delete_list.push_back(elp);
    
    elp->Draw("same");
    
    TEllipse* elp_rms = new TEllipse(pk_x,pk_y,r_rms,r_rms);
    elp_rms->SetFillStyle(0);
    elp_rms->SetLineStyle(9);
    if(_mini_cluster_list[i]->IsSeedPk()){
      elp_rms->SetLineColor(kRed);
    }
    _delete_list.push_back(elp_rms);

    elp_rms->Draw("same");

  }

  c->cd(2);
  hsquare_2d_w_e->Draw("colz");
  
  TFile* ofile = NULL;
  string ofname = string("Visual_MiniClusters_")+string(dataset)+string(".root");
  if(ct>0) ofile = new TFile(ofname.c_str(),"UPDATE");
  else{
    gSystem->Exec(Form("rm -f %s",ofname.c_str()));
    ofile = new TFile(ofname.c_str(),"RECREATE"); 
  }

  c->Write();
  ofile->Close();

  for(unsigned i=0;i<_delete_list.size();i++){
    delete _delete_list[i];
    _delete_list[i] = NULL;
  }
  _delete_list.clear();

  ct++;
}

bool ExMiniClusters::IsGoodSq(Square* sq){
  if(!sq) return false;
  //continues cut
  bool e_start = false;
  for(int i=0;i<8;i++){
    if(e_start && sq->GetLayerE(i)<=0.0000001){
      return false;
    } 
    if(sq->GetLayerE(i)>0) e_start = true;
  }
  return true;
}
