#include <vector>
#include <string>

void TestToHDF5(){
  gSystem->Load("libMyUtility.so");

  ToHDF5* toh5 = new ToHDF5();
  
  TH1D* hshower_e = new TH1D("hshower_e","shower_e",200,0,100);
  hshower_e->GetXaxis()->SetTitle("E/GeV");

  float shower_e;
  float mpcE3x3;
  float towerE[3][3];
  float hit2d[8][65][65];
  int evt_num;
  vector<int> dims;
//  dims.push_back(1);
  toh5->AddVar("shower_e","Float",&shower_e,dims);
  toh5->AddVar("mpcE3x3","Float",&mpcE3x3,dims);
  toh5->AddVar("evt_num","Int",&evt_num,dims);
  dims.push_back(3);
  dims.push_back(3);
  toh5->AddVar("tower3x3E","Float",towerE,dims);

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
  
  int tot_showers=0;
 
  for(int i=0;i<entries;i++){
    evtTree->GetEntry(i);
    int nshowers = evt->GetNShowers();
    double vertex = evt->GetVertex();
    if(fabs(vertex)>30) continue;
    
    for(int iswr=0;iswr<nshowers;iswr++){
      ExShowerV2* ex_shower = dynamic_cast<ExShowerV2*>(evt->GetExShower(iswr));
      int arm = ex_shower->GetArm();
      if(arm==0) continue;
      toh5->Set2DHits(ex_shower);
      shower_e = ex_shower->GetTotalE();
      hshower_e->Fill(shower_e);
      evt_num=i;
      mpcE3x3 = ex_shower->GetMpcE3x3();
      towerE[1][1]=10;
      toh5->Fill();
      tot_showers++;
    }
  }

  cout<<"number of showers: "<<tot_showers<<endl;
  /*
  H5ToRoot* h2r = new H5ToRoot("Ex.h5"); 
  h2r->AddVar("shower_e");
  h2r->AddVar("mpcE3x3");
  h2r->AddVar("2d_hits");
  h2r->AddVar("evt_num");
  h2r->AddVar("tower3x3E");
  h2r->ToRoot(string("test.root"));
  */
//  hshower_e->Draw();
}
