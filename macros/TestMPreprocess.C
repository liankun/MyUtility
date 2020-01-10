#include "../MIndexing.h"
#include <fstream>

class MTensor;
class ExShower;

void VisualExTensor(MTensor* h);
void VisualExShower(ExShower* shower);

const double DSPACE=40./214.;
const double SHOWERSIZE=6;

void TensorToTxT(MTensor* h,ofstream& out);
void TensorToTxT(MTensor* t);

void TestMPreprocess(){
  gSystem->Load("libMyUtility.so");
  MModelV1* model_v1 = new MModelV1();

  MPreprocess* prep = new MPreprocess();

  ExSimEventV3* evt = new ExSimEventV3();
  TFile* ifile = new TFile("/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/embeding/macros/condor_hijing/output_single_gamma_all/output_0/ExHijingSTPi0.root","READONLY");
  if(ifile->IsZombie()){
    cout<<"Open file failed !"<<endl;
    return;
  }

  TTree* evtTree = (TTree*)ifile->Get("ExHijingSTPi0Tree");
  if(!evtTree){
    cout<<"No ExEvtTree !"<<endl;
    return;
  }

  evtTree->SetBranchAddress("ExSimEventV3",&evt);
  int entries = evtTree->GetEntries();
  cout<<"Number of the entries: "<<entries<<endl;
  entries = 100;
  MIndex idx(3,65);
  idx[2]=8;
  MTensor* mt = new MTensor(idx);

  ofstream out_txt("tensor_out_example.txt");
  for(int i=0;i<entries;i++){
    evtTree->GetEntry(i);
    int nshowers=evt->GetNShowers();
    double vertex = evt->GetVertex();
    if(fabs(vertex)>30) continue;
    for(int iswr=0;iswr<nshowers;iswr++){
      ExShowerV2* ex_shower = dynamic_cast<ExShowerV2*>(evt->GetExShower(iswr));
      int arm = ex_shower->GetArm();
      if(arm==0) continue;
//      cout<<"Number of hits: "<<ex_shower->GetNhits()<<endl;
      bool is_valid=prep->SetTensor(mt,ex_shower); 
      if(!is_valid){
        cout<<"tensor set invalid !"<<endl;
	continue;
      }

      TensorToTxT(mt,out_txt);
      
      /*
      cout<<"Model result:"<<endl;
      vector<double> prob = model_v1->GetProb(mt);
      for(unsigned int ip=0;ip<prob.size();ip++){
        cout<<prob[ip]<<endl;
      }
      */
//      TensorToTxT(mt);
      
//      VisualExTensor(mt);
//      VisualExShower(ex_shower);
      //cout<<"first tensor element: "<<mt->GetValue(i)<<endl;
//      mt->Print1DTensor();
    }
  }

  out_txt.close();
}

void VisualExTensor(MTensor* t){
  static int ct=0;
  
  TH3D* htensor = new TH3D("htensor","Tensor",65,-6,6,65,-6,6,8,-0.5,7.5);
  htensor->GetXaxis()->SetTitle("X/cm");
  htensor->GetYaxis()->SetTitle("Y/cm");
  htensor->GetZaxis()->SetTitle("layer");
//  t->Print1DTensor();
  MIndex idx(3,0);
  for(unsigned int i=0;i<65;i++){
    for(unsigned int j=0;j<65;j++){
      for(unsigned int k=0;k<8;k++){
        idx[0]=i;
	idx[1]=j;
	idx[2]=k;
        double val =t->GetValue(idx);
	double x = i*DSPACE+DSPACE/2.-SHOWERSIZE;
	double y = j*DSPACE+DSPACE/2.-SHOWERSIZE;
	double z = k;
	int i_bin = htensor->GetBin(i+1,j+1,k+1);
//	htensor->AddBinContent(i_bin,val);
//	if(k==0 || k==1) val=val*10;
//	htensor->Fill(x,y,z,val);
	htensor->SetBinContent(i+1,j+1,k+1,val*100);
	
	if(k==0 || k==1){
//	  if(val>0) cout<<i<<"  "<<j<<"  "<<k<<"  "<<val<<endl;
	}
      }
    }
  }

  TCanvas* c = new TCanvas("c","c",1500,1500/2);
  c->Divide(4,2);
  for(unsigned int i=0;i<8;i++){
    c->cd(i+1);
    htensor->SetAxisRange(i,i,"z");
    TH2D* h2d = htensor->Project3D("yx");
    h2d->SetName(Form("htsr_2d_%d_ct%d",i,ct));
    h2d->Draw("colz");
  }
  c->Print(Form("htensor_vs_%d.gif",ct));
 
  ct++;
}

void VisualExShower(ExShower* shower){
  static int ct=0;
  Exogram* hexo = new Exogram("hexo","hexo",900,-20,20,900,-20,20,8,-0.5,7.5);
//  cout<<"Number of hits: "<<shower->GetNhits()<<endl; 
  double raw_ex_e = shower->GetRawMpcexE()*1.0e6;
  double sum_e=0;
  for(unsigned int i=0;i<shower->GetNhits();i++){
    ExHit* hit = shower->GetExHit(i);
    int key = hit->GetKey();
    sum_e+=hit->GetE();
    double e = hit->GetE()/raw_ex_e;
    hexo->FillEx(key,e*100);
  }

  cout<<"sum e ratio: "<<sum_e/raw_ex_e<<endl;

  TCanvas* c = new TCanvas("c_exo","c_exo",1500,1500/2);
  c->Divide(4,2);
  double vertex = shower->GetVertex();
  double z=205;
  if(shower->GetArm()==0) z=-205;
  double ct_x = shower->GetHsx()*(z-vertex);
  double ct_y = shower->GetHsy()*(z-vertex); 
  
  hexo->SetAxisRange(ct_x-6,ct_x+6,"x");
  hexo->SetAxisRange(ct_y-6,ct_y+6,"y");

  for(unsigned int i=0;i<8;i++){
    c->cd(i+1);
    hexo->SetAxisRange(i,i,"z");
    TH2D* h2d = hexo->Project3D("yx");
    h2d->SetName(Form("hexo_2d_%d_ct%d",i,ct));
    h2d->Draw("colz");
  }
  c->Print(Form("hexo_vs_%d.gif",ct));
  ct++;
}

void TensorToTxT(MTensor* t,ofstream& out){
  unsigned int nsize = t->GetVolume();
  double sum_val=0;
  for(unsigned int i=0;i<nsize;i++){
    out<<t->GetValue(i)<<" ";
    sum_val+=t->GetValue(i);
  }
  out<<endl;
//  cout<<"Sum of all hits E: "<<sum_val<<endl;
}


void TensorToTxT(MTensor* t){
  static int ct=0;
  ofstream tmp_out_txt(Form("one_tensor_index_value_ct%d.txt",ct));
  MIndex index(3,0);
  for(unsigned int i=0;i<65;i++){
    for(unsigned int j=0;j<65;j++){
      for(unsigned int k=0;k<8;k++){
        index[0]=i;
	index[1]=j;
	index[2]=k;
	double value = t->GetValue(index);
	tmp_out_txt<<i<<" "<<j<<" "<<k<<" "<<value<<endl;
      }
    }
  }
  ct++;
  tmp_out_txt.close();
}
