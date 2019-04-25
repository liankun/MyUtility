void draw_mpcex(){
  gSystem->Load("libMyUtility.so");
  Exogram* hexo[2];

  hexo[0] = new Exogram("hexo0","Exogram South Arm",900,-19.5,19.5,900,-19.5,19.5,8,-0.5,7.5);
  hexo[1] = new Exogram("hexo1","Exogram North Arm",900,-19.5,19.5,900,-19.5,19.5,8,-0.5,7.5);
  
  MpcExMapper* ex_map = MpcExMapper::instance();
  ExAcpt* ex_acpt = ExAcpt::instance();
  for(int i=0;i<128*24*16;i++){
    int arm = ex_map->get_arm(i);
    int layer = ex_map->get_layer(i);
    double x = ex_map->get_x(i);
    double y = ex_map->get_y(i);
    double d = ex_acpt->get_shortest_d(arm,layer,x,y);
    if(d>1){
      hexo[arm]->FillEx(i,2);
    }
    else hexo[arm]->FillEx(i,1);

  }

  TFile* ofile = new TFile("Test.root","RECREATE");
  hexo[0]->Write();
  hexo[1]->Write();
}
