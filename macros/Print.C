void Print(){
  gSystem->Load("libMyUtility.so");
  ExAcpt* ex_acpt = ExAcpt::instance();
  ex_acpt->print_data(0);
}
