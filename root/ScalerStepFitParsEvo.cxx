double Ampl(double hv){
  double p0 = 29233.9;
  double p1 = -322.825;
  double p2 = 0.93872;
  double Y = p0 + p1*hv + p2*hv*hv;
  return Y;
}

double PL_Base(double hv){
  double p0 = 1.24805;
  double p1 = -0.00283842;
  double Y = p0 + p1*hv;
  return Y;
}

double Period(double hv){
  double p0 =  76.1833;
  double p1 = -0.301605;
  double Y = p0 + p1*hv;
  return Y;
}

double CosAmpl(double hv){
  double p0 = -0.375287;
  double p1 = 0.00303766;
  double Y = p0 + p1*hv;
  return Y;
}

double Phase(double hv){
  double p0 = 14.8124;
  double p1 = -0.0253561;
  double Y = p0 + p1*hv;
  return Y;
}

void PlotScalerSteps(const char* root_file, const char* root_dir, const char* root_tree){

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

  double Chi[70], dChi[70], PE[70], dPE[70], AmpPL[70], dAmpPL[70],
         AmpCos[70], dAmpCos[70], XoffCos[70], dXoffCos[70], HV[70], maxFreq[70];

  //------------ FIT FUNCTION ----------------//
  char eqn[100];
  sprintf(eqn,"[0]*TMath::Power([1],2*3.141592/[2]*x)*(1+[3]*TMath::Cos(2*3.141592/[2]*(x-[4])))");
  float diffBeg = 9999., diffEnd = 9999., fBeg=750, fEnd=10;
  int fitBeg, fitEnd;
  double A, B, C, D, E;

  TH1I *hist = new TH1I("hist","S10362-013-050C Trigger Scalers", 150, 0, 150);
  for (int hv=130; hv<200; hv++){
    tree->GetEntry(hv);
    int i = hv-130;
//    HV[i] = (double)approxHV;
    HV[i] = (double)hv;

    for(int j=0; j<150; j++){
      hist->SetBinContent(j, freq[j]);
    }

    //------------ FITTING DATA ----------------//
    diffBeg = 9999.; diffEnd = 9999.;
    for(int k=0; k<150; k++){
      if (TMath::Abs(freq[k]-fBeg) < diffBeg){
        fitBeg=k;
        diffBeg = TMath::Abs(freq[k]-fBeg);
      }
      if (TMath::Abs(freq[k]-fEnd) < diffEnd){
        fitEnd=k;
        diffEnd = TMath::Abs(freq[k]-fEnd);
      }
    }
    TF1* fit = new TF1("fit", eqn, fitBeg, fitEnd);
    A = Ampl(hv);
    B = PL_Base(hv);
    C = Period(hv);
    D = CosAmpl(hv);
    E = Phase(hv);
    fit->SetParameters(A,B,C,D,E);
    hist->Fit("fit", "RS");

    AmpPL[i]    = fit->GetParameter(0);
    dAmpPL[i]   = fit->GetParError(0);
    Chi[i]      = fit->GetParameter(1);
    dChi[i]     = fit->GetParError(1);
    PE[i]       = fit->GetParameter(2);
    dPE[i]      = fit->GetParError(2);
    AmpCos[i]   = fit->GetParameter(3);
    dAmpCos[i]  = fit->GetParError(3);
    XoffCos[i]  = fit->GetParameter(4);
    dXoffCos[i] = fit->GetParError(4);

    delete fit;
  }

  //-------- PLOT RESULTS -------------//
  gStyle->SetOptStat(1);
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1100, 850);
  canv->Divide(3,2);
  TGraphErrors* gr0 = new TGraphErrors(70, HV, AmpPL, 0, dAmpPL);
  TGraphErrors* gr1 = new TGraphErrors(70, HV, Chi, 0, dChi);
  TGraphErrors* gr2 = new TGraphErrors(70, HV, PE, 0, dPE);
  TGraphErrors* gr3 = new TGraphErrors(70, HV, AmpCos, 0, dAmpCos);
  TGraphErrors* gr4 = new TGraphErrors(70, HV, XoffCos, 0, dXoffCos);

  canv->cd(1);
  gr0->SetMaximum(4000);
  gr0->SetMinimum(0);
  gr0->Draw("ap");
  gr0->SetMarkerStyle(20);
  gr0->SetTitle("Power-Law Amplitude vs. HV Trim");
  gr0->Fit("pol2", "R", "", 130, 180);

  canv->cd(2);
  gr1->SetMaximum(1);
  gr1->SetMinimum(0);
  gr1->Draw("ap");
  gr1->SetMarkerStyle(20);
  gr1->SetTitle("Power-Law Base vs. HV Trim");
  gr1->Fit("pol1", "R", "", 130, 180);

  canv->cd(3);
  gr2->SetMaximum(50);
  gr2->SetMinimum(0);
  gr2->Draw("ap");
  gr2->SetMarkerStyle(20);
  //gr2->SetTitle("Cosine Period vs. HV Trim");
  gr2->SetTitle("Cosine Period vs. HV");
  //gr2->Fit("pol1", "R", "", 130, 180);
  gr2->Fit("pol1", "R", "", 70.9, 72);

  canv->cd(4);
  gr3->SetMaximum(1);
  gr3->SetMinimum(0);
  gr3->Draw("ap");
  gr3->SetMarkerStyle(20);
  gr3->SetTitle("Cosine Amplitude vs. HV Trim");
  gr3->Fit("pol1", "R", "", 135, 165);

  canv->cd(5);
  gr4->SetMaximum(50);
  gr4->SetMinimum(-50);
  gr4->Draw("ap");
  gr4->SetMarkerStyle(20);
  gr4->SetTitle("Cosine Phase vs. HV Trim");
  gr4->Fit("pol1", "R", "", 130, 200);


  //canv->Update();

}
