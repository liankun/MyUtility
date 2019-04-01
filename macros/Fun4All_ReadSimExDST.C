void Fun4All_ReadSimExDST(int nfile=0) {

  std::string dstfile = "simDST.root";
//  int runNumber = 429506;
  
  int runNumber = 455450;

  gSystem->Load("libfun4all");
  gSystem->Load("libfun4allfuncs.so");
  gSystem->Load("libbbc.so");
  gSystem->Load("libt0.so");
//  gSystem->Load("libmpc.so");
  gSystem->Load("libmpcex_base.so");
  gSystem->Load("libmpcex_interface.so");
  gSystem->Load("libmpcex_modules.so");
  gSystem->Load("libmpcex_utils.so");
  gSystem->Load("libMyUtility.so");
  //  gSystem->Load("libjprof.so");
  //  prof *pr = new prof();

  //  getchar();
  
  ///////////////////////////////////////////
  //
  //- Make the Server
  //
  //////////////////////////////////////////
  se = Fun4AllServer::instance();

  recoConsts *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER",runNumber);
  rc->set_IntFlag("SIMULATIONFLAG",0x1);
  ///////////////////////////////////////////
  //
  //- Subsystems
  //
  //////////////////////////////////////////

//  rc->set_IntFlag("MPC_RECO_MODE",0x16);
//  MpcReco *mpc = new MpcReco();
//  se->registerSubsystem(mpc);
  
  //sim but use deadhot channel
  rc->set_IntFlag("MPCEXCALIBMODE",0x3);
  
  
  mMpcExCreateNodeTree *noder = new mMpcExCreateNodeTree();
  se->registerSubsystem(noder);

  mMpcExMyDigitizeHits *digitizer = new mMpcExMyDigitizeHits();
  se->registerSubsystem(digitizer);

  mMpcExSimEventHeader *header = new mMpcExSimEventHeader();
  se->registerSubsystem(header);

  mMpcExLoadCalibrations *calibreader = new mMpcExLoadCalibrations();
  se->registerSubsystem(calibreader);

  SubsysReco *calibdoer = new mMpcExMyApplyCalibrations();
  se->registerSubsystem(calibdoer);

  /*
  mMpcExShower* shower = new mMpcExShower();
  shower->DisableSouthReco();
  shower->DisableSTReco();
  se->registerSubsystem(shower);

  SubsysReco* cleaner = new mMpcExShowerCleaner();
  se->registerSubsystem(cleaner);


  SubsysReco* simAna = new ExSimAna();
  se->registerSubsystem(simAna);
  */

  ///////////////////////////////////////////
  //
  //- Analyze the Data.
  //
  //////////////////////////////////////////
  

  Fun4AllInputManager *simdst = new Fun4AllDstInputManager( "SIM", "DST", "TOP");
//  simdst->AddFile(dstfile);
  se->registerInputManager(simdst);

  string filepath;

//  ifstream myfile("sim_no_pi0_acpt_cut.txt");
  ifstream myfile("hijing_list_new_all.txt");
  int file_counts = 0;
  if(myfile.is_open()){

    gBenchmark->Start("showery");  
 
    while(getline(myfile,filepath)){
      string tfilepath = filepath;
      
      if(file_counts > 10){
        file_counts++;
	break;
      }

      cout <<"process "<<tfilepath<<endl;
      se->fileopen(simdst->Name(),tfilepath.c_str());
      se->run(0);
      
      file_counts++;
    }

    se->End();
    gBenchmark->Show("showery");  


  }
  else cout<<"file open failed !!!"<<endl;
}
