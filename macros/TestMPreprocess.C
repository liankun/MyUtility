#include "../MIndexing.h"
class MTensor;
class ExShower;

void VisualExTensor(MTensor* h);
void VisualExShower(ExShower* shower);

const float DSPACE=40./214.;
const float SHOWERSIZE=6;

void TestMPreprocess(){
  gSystem->Load("libMyUtility.so");
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
  entries = 10;
  MIndex idx(3,65);
  idx[2]=8;
  MTensor* mt = new MTensor(idx);
  for(int i=0;i<entries;i++){
    evtTree->GetEntry(i);
    int nshowers=evt->GetNShowers();
    double vertex = evt->GetVertex();
    if(fabs(vertex)>30) continue;
    for(int iswr=0;iswr<nshowers;iswr++){
      ExShowerV2* ex_shower = dynamic_cast<ExShowerV2*>(evt->GetExShower(iswr));
      int arm = ex_shower->GetArm();
      if(arm==0) continue;
      cout<<"Number of hits: "<<ex_shower->GetNhits()<<endl;
      bool is_valid=prep->SetTensor(mt,ex_shower); 
      if(!is_valid){
        cout<<"tensor set invalid !"<<endl;
      }
      VisualExTensor(mt);
      VisualExShower(ex_shower);
      //cout<<"first tensor element: "<<mt->GetValue(i)<<endl;
//      mt->Print1DTensor();
    }
  }
}

void VisualExTensor(MTensor* t){
  static int ct=0;
  TH3F* htensor = new TH3F("htensor","Tensor",65,-6,6,65,-6,6,8,-0.5,7.5);
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
        float val =t->GetValue(idx);
	float x = i*DSPACE+DSPACE/2.-SHOWERSIZE;
	float y = j*DSPACE+DSPACE/2.-SHOWERSIZE;
	float z = k;
//	if(k==0 || k==1) val=val*10;
//	htensor->Fill(x,y,z,val);
	htensor->SetBinContent(i+1,j+1,k+1,val);
	
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
  float raw_ex_e = shower->GetRawMpcexE()*1.0e6;
  for(unsigned int i=0;i<shower->GetNhits();i++){
    ExHit* hit = shower->GetExHit(i);
    int key = hit->GetKey();
    float e = hit->GetE()/raw_ex_e;
    hexo->FillEx(key,e);
  }

  TCanvas* c = new TCanvas("c_exo","c_exo",1500,1500/2);
  c->Divide(4,2);
  float vertex = shower->GetVertex();
  float z=205;
  if(shower->GetArm()==0) z=-205;
  float ct_x = shower->GetHsx()*(z-vertex);
  float ct_y = shower->GetHsy()*(z-vertex); 
  
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
