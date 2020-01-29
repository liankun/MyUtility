void TestToHDF5(){
  gSystem->Load("libMyUtility.so");

  ToHDF5* toh5 = new ToHDF5();

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
 
  for(int i=0;i<10;i++){
    evtTree->GetEntry(i);
    int nshowers = evt->GetNShowers();
    double vertex = evt->GetVertex();
    if(fabs(vertex)>30) continue;
    
    for(int iswr=0;iswr<nshowers;iswr++){
      ExShowerV2* ex_shower = dynamic_cast<ExShowerV2*>(evt->GetExShower(iswr));
      int arm = ex_shower->GetArm();
      if(arm==0) continue;
      toh5->FillDataSet(ex_shower);
      tot_showers++;
    }
  }

  cout<<"number of showers: "<<tot_showers<<endl;
}
