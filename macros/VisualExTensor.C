void VisualExTensor(){
  TH3D* htensor = new TH3D("htensor","Tensor Visual",65,-6,6,65,-6,6,8,-0.5,7.5);
  htensor->GetXaxis()->SetTitle("X/cm");
  htensor->GetYaxis()->SetTitle("Y/cm");
  htensor->GetZaxis()->SetTitle("Layer");



}
