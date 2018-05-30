void PlotScalerSteps(const char* root_file, const char* root_dir, const char* root_tree, const int hvTrim){


  //--------- DECLARE ROOT FILE & TREE ----------#
  TFile* file = new TFile(root_file, "UPDATE");
  if (!gFile->Get(root_dir)) {
    printf("\033[31mCan't find directory %s in %s . . . exiting!\033[0m\n", root_dir, root_file);
    exit(-1);
  }

  file->cd(root_dir);
  if (!gDirectory->Get(root_tree)){
    printf("\033[31mCan't find TTree %s in %s . . . exiting!\033[0m\n", root_tree, root_dir);
    exit(-1);
  }

  TTree* tree = (TTree*)gDirectory->Get(root_tree);
  float freq[150];  tree->SetBranchAddress("trigFreq", freq);
  int   HVtrim;     tree->SetBranchAddress("HVtrimDAC", &HVtrim);
  float approxHV;   tree->SetBranchAddress("approxHV", &approxHV);
  tree->GetEntry(hvTrim);

  //-------- CANVASES & GRAPHS -------------//
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();
  canv->Divide(2,2);

  TH1I *gr1 = new TH1I("gr1","S10362-013-050C Trigger Scalers", 150, 0, 150);
  for(int i=5; i<150; i++){
    gr1->SetBinContent(i, freq[i]);
  }

  canv->cd(1);
  gr1->SetMarkerStyle(20);
  gr1->Draw("P");
  gr1->GetXaxis()->SetTitle( "Trigger Level (trig DAC counts)" );
  gr1->SetMaximum(1000);
  gr1->GetYaxis()->SetTitle( "Trigger Scaler Frequency (kHz)" );
  gr1->GetYaxis()->SetTitleOffset(1.5);
  canv->Update();

  //------------ FITTING DATA ----------------//
  char eqn[100];
  sprintf(eqn,"[0]*TMath::Power([1],2*3.141592/[2]*x)*(1+[3]*TMath::Cos(2*3.141592/[2]*(x-[4])))");
  float diffLow = 9999., diffHigh = 9999.;
  int fitLow, fitHigh;
  for(int i=0; i<150; i++){
    if (TMath::Abs(freq[i]-750) < diffLow){
      fitLow=i;
      diffLow = TMath::Abs(freq[i]-750);
    }
    if (TMath::Abs(freq[i]-10) < diffHigh){
      fitHigh=i;
      diffHigh = TMath::Abs(freq[i]-10);
    }
  }
  TF1* PwrLaw = new TF1("PwrLaw", eqn, fitLow, fitHigh);
  PwrLaw->SetParameters(1257.6,.3679,20,.166, 7.24);
  gr1->Fit("PwrLaw", "RS");
  canv->Update();
  //canv->Print("testScalerSteps.pdf");

  //------------- RESIDUALS ---------------------//
  canv->cd(2);
  TH1I *gr2 = new TH1I("gr2","Residuals", 150, 0, 150);
  for(int i=5; i<150; i++){
    gr2->SetBinContent(i, PwrLaw->Eval(i)-freq[i]);
  }
  gr2->SetMarkerStyle(20);
  gr2->Draw("LP");
  gr2->GetXaxis()->SetTitle( "Trigger Level (trig DAC counts)" );
  gr2->SetMaximum(30);
  gr2->SetMinimum(-30);
  gr2->GetYaxis()->SetTitle( "Fit Residuals (kHz)" );
  gr2->GetYaxis()->SetTitleOffset(1.5);
  canv->Update();

  //------------- DERIVATIVE ---------------------//
  canv->cd(3);
  TH1I *gr3 = new TH1I("gr3","1st Derivative of Trigger Scalers", 150, 0, 150);
  TH1F *cl3 = (TH1F*)gr3->Clone("cl3");
  for(int i=5; i<150; i++){
    gr3->SetBinContent(i, freq[i]-freq[i-1]);
    cl3->SetBinContent(i, PwrLaw->Derivative(i));
  }
  gr3->SetMarkerStyle(20);
  gr3->Draw("P");
  cl3->Draw("L SAMES");
  cl3->SetLineColor(3);
  cl3->SetLineWidth(4);
  gr3->GetXaxis()->SetTitle( "Trigger Level (trig DAC counts)" );
  gr3->SetMaximum(20);
  gr3->SetMinimum(-80);
  gr3->GetYaxis()->SetTitle( "Frequency Difference w.r.t Trim DAC (kHz)" );
  gr3->GetYaxis()->SetTitleOffset(1.5);
  canv->Update();

  //------------- COSINE TERM ---------------------//
  canv->cd(1);
  TF1 *CosTerm = new TF1("CosTerm","100*TMath::Cos(2*3.141592/[0]*(x-[1]))+500",0,150);
  CosTerm->SetParameters(PwrLaw->GetParameter(2), PwrLaw->GetParameter(4));
  CosTerm->Draw("SAMES");
  CosTerm->SetLineColor(4);
//  gr4->GetXaxis()->SetTitle( "Trigger Level (trig DAC counts)" );
//  gr4->SetMaximum(30);
//  gr4->SetMinimum(-30);
//  gr4->GetYaxis()->SetTitle( "Fit Residuals (kHz)" );
//  gr4->GetYaxis()->SetTitleOffset(1.5);
  canv->Update();

  canv->cd(4);
  char eqn2[150];
  int h = 440;
  int w = 440;
  int a = 20;
  sprintf(eqn2,"-%d*(1/2*1/TMath::TanH(%d/2)*TMath::TanH(%d*((x/%d - TMath::Floor(x/%d)) - 0.5)) + 1/2 + TMath::Floor(x/%d))",h,a,a,w,w,w);
  TF1 *SmoothStairs = new TF1("SmoothStairs",eqn2,0,1200);
  SmoothStairs->Draw();
  canv->Update();
  //double r[4] = {PwrLaw->GetParameter(1), PwrLaw->GetParError(1), PwrLaw->GetParameter(2), PwrLaw->GetParError(2)};


}
