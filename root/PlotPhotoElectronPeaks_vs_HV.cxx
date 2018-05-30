// functions declared here, used in next section/plot
double YofPE(double m, double b, int n, double x0, double y0, double x){
  double slope = m*n + b;
  double Y = slope*(x-x0) + y0;
  return Y;
}
double dYofPE(double dm,double db, int n){
  double dslope = TMath::Sqrt(dm*dm*n*n + db*db);
  return dslope;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// PHOTOELECTRON PEAK EVOLUTION /////////////////////////
////////////////////////////////////////////////////////////////////////////////
float PlotPhotoElectronPeaks_vs_HV(const char *root_file, const char *root_dir, const int argASIC, const int argCH) {
  gStyle->SetOptFit();
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  // measurements TTree variables
  Int_t asic, channel;
  Double_t SatPoint, ADCperTrigDAC, dADCperTrigDAC;

  TFile* file = new TFile(root_file,"UPDATE");

  TTree* measurements = (TTree*)file->Get("measurements");
  measurements->SetBranchAddress("ASIC", &asic);
  measurements->SetBranchAddress("Channel", &channel);
  measurements->SetBranchAddress("SatPoint", &SatPoint);
  measurements->SetBranchAddress("ADCperTrigDAC", &ADCperTrigDAC);
  measurements->SetBranchAddress("dADCperTrigDAC", &dADCperTrigDAC);

  int theEntry=-1;
  for (int e=0; e<measurements->GetEntriesFast(); e++){
    measurements->GetEntry(e);
    if (asic==argASIC && channel==argCH){
      int theEntry = e;
    }
  }
  measurements->GetEntry(theEntry);




  TDirectory *dir;
  gDirectory->pwd();
  if (file->cd(root_dir)) {
    dir = (TDirectory*)file->Get(root_dir);
  }
  else {
    printf("Can't find directory %s in %s . . . exiting!\n", root_dir, root_file);
    exit(-1);
  }
  file->cd(root_dir);//dir->cd();
  gDirectory->pwd();

  // PEvsHV TTree variables
  Double_t Mean[5], dMean[5], Width[5], dWidth[5];
  Float_t HV;

  TTree* tree = (TTree*)dir->Get("PEvsHV");
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("dMean", dMean);
  tree->SetBranchAddress("Width", Width);
  tree->SetBranchAddress("dWidth", dWidth);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t mean1[NumEnt], mean2[NumEnt], mean3[NumEnt], mean4[NumEnt], mean5[NumEnt];
  Double_t dmean1[NumEnt], dmean2[NumEnt], dmean3[NumEnt], dmean4[NumEnt], dmean5[NumEnt];

  Double_t width1[NumEnt], width2[NumEnt], width3[NumEnt], width4[NumEnt], width5[NumEnt];
  Double_t dwidth1[NumEnt], dwidth2[NumEnt], dwidth3[NumEnt], dwidth4[NumEnt], dwidth5[NumEnt];

  Double_t p1Norm[NumEnt], p2Norm[NumEnt], p3Norm[NumEnt], p4Norm[NumEnt], p5Norm[NumEnt];
  Double_t dp1Norm[NumEnt], dp2Norm[NumEnt], dp3Norm[NumEnt], dp4Norm[NumEnt], dp5Norm[NumEnt];

  Double_t hv[NumEnt];

  Double_t Gain[NumEnt], dGain[NumEnt];

  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    mean1[i] = Mean[0]; dmean1[i] = dMean[0];
    mean2[i] = Mean[1]; dmean2[i] = dMean[1];
    mean3[i] = Mean[2]; dmean3[i] = dMean[2];
    mean4[i] = Mean[3]; dmean4[i] = dMean[3];
    mean5[i] = Mean[4]; dmean5[i] = dMean[4];
    width1[i] = Width[0]; dwidth1[i] = dWidth[0];
    width2[i] = Width[1]; dwidth2[i] = dWidth[1];
    width3[i] = Width[2]; dwidth3[i] = dWidth[2];
    width4[i] = Width[3]; dwidth4[i] = dWidth[3];
    width5[i] = Width[4]; dwidth5[i] = dWidth[4];
    Gain[i] = dGain[i] = 0.0;
    for (int j=0; j<4; j++){
      for (int k=j+1; k<4; k++){
        // 10 avg's, e- charge given in fC.
        Gain[i] += (Mean[k]-Mean[j]) / (6.*1.60217662e-4*(k-j));
      }
    }
    for (int j=0; j<4; j++){
      for (int k=j+1; k<4; k++){
        // Std. Dev. of mean squared.
        dGain[i] += TMath::Power((Mean[k]-Mean[j]) / (1.60217662e-4*(k-j)) - Gain[i], 2)/5.;
      }
    }
    dGain[i] = TMath::Sqrt(dGain[i]);
    cout << "Gain " << i << ": " << Gain[i] << " +/- " << dGain[i];
    p1Norm[i] = mean1[i]/1; dp1Norm[i] = dmean1[i]/1;
    p2Norm[i] = mean2[i]/2; dp2Norm[i] = dmean2[i]/2;
    p3Norm[i] = mean3[i]/3; dp3Norm[i] = dmean3[i]/3;
    p4Norm[i] = mean4[i]/4; dp4Norm[i] = dmean4[i]/4;
    p5Norm[i] = mean5[i]/5; dp5Norm[i] = dmean5[i]/5;
    hv[i] = (Double_t)HV;
  }

  // fit functions
  TF1 *f0 = new TF1("f0", "pol1");

  TMultiGraph *mgA = new TMultiGraph();
  TGraphErrors *gr1 = new TGraphErrors(NumEnt, hv, mean1, 0, dmean1);
  TGraphErrors *gr2 = new TGraphErrors(NumEnt, hv, mean2, 0, dmean2);
  TGraphErrors *gr3 = new TGraphErrors(NumEnt, hv, mean3, 0, dmean3);
  TGraphErrors *gr4 = new TGraphErrors(NumEnt, hv, mean4, 0, dmean4);
  TGraphErrors *gr5 = new TGraphErrors(NumEnt, hv, mean5, 0, dmean5);


  Double_t slope[5], dslope[5], Yint[5], dYint[5];

  gr1->SetMarkerColor(2); gr1->SetTitle("1 PE"); gr1->SetMarkerStyle(20);;
  gr1->Fit("f0","FS", "SAMES");
  Yint[0]  = f0->GetParameter(0); dYint[0]  = f0->GetParError(0);
  slope[0] = f0->GetParameter(1); dslope[0] = f0->GetParError(1);
  gr1->GetFunction("f0")->SetLineColor(1); mgA->Add(gr1);

  gr2->SetMarkerColor(3); gr2->SetTitle("2 PE"); gr2->SetMarkerStyle(20);
  gr2->Fit("f0","FS", "SAMES");
  Yint[1]  = f0->GetParameter(0); dYint[1]  = f0->GetParError(0);
  slope[1] = f0->GetParameter(1); dslope[1] = f0->GetParError(1);
  gr2->GetFunction("f0")->SetLineColor(1); mgA->Add(gr2);

  gr3->SetMarkerColor(4); gr3->SetTitle("3 PE"); gr3->SetMarkerStyle(20);
  gr3->Fit("f0","FS", "SAMES");
  Yint[2]  = f0->GetParameter(0); dYint[2]  = f0->GetParError(0);
  slope[2] = f0->GetParameter(1); dslope[2] = f0->GetParError(1);
  gr3->GetFunction("f0")->SetLineColor(1); mgA->Add(gr3);

  gr4->SetMarkerColor(6); gr4->SetTitle("4 PE"); gr4->SetMarkerStyle(20);
  gr4->Fit("f0","FS", "SAMES");
  Yint[3]  = f0->GetParameter(0); dYint[3]  = f0->GetParError(0);
  slope[3] = f0->GetParameter(1); dslope[3] = f0->GetParError(1);
  gr4->GetFunction("f0")->SetLineColor(1); mgA->Add(gr4);

  gr5->SetMarkerColor(kOrange-3); gr5->SetTitle("5 PE"); gr5->SetMarkerStyle(20);
  gr5->Fit("f0","FS", "SAMES");
  Yint[4]  = f0->GetParameter(0); dYint[4]  = f0->GetParError(0);
  slope[4] = f0->GetParameter(1); dslope[4] = f0->GetParError(1);
  gr5->GetFunction("f0")->SetLineColor(1); mgA->Add(gr5);

  //Calculate avg X & Y intersection point
  Double_t twiceX0sum, twiceY0sum, x0avg, y0avg;
  twiceX0sum = twiceY0sum = 0;
  int NumPeaks = 4;
  for (int i=0; i<NumPeaks; i++){
    for (int j=0; j<NumPeaks; j++){
      if (i!=j){
        twiceX0sum+= (Yint[j]-Yint[i]) / (slope[i]-slope[j]);
        twiceY0sum+= (slope[j]*Yint[i]-slope[i]*Yint[j]) / (slope[j]-slope[i]);
      }
    }
  }
  x0avg = 1/((double)(NumPeaks*(NumPeaks-1)))*twiceX0sum;
  y0avg = 1/((double)(NumPeaks*(NumPeaks-1)))*twiceY0sum;
cout << "\n\nX-intersect average: " << x0avg << "\nY-intersect average: " << y0avg << "\n\n";

  mgA->Draw("ap");

  mgA->SetTitle("Photo-Electron Peaks");
  mgA->SetMinimum(0);
  mgA->SetMaximum(5000);
  mgA->GetYaxis()->SetTitle("Mean Integrated ADC Counts");
  mgA->GetYaxis()->SetLabelSize(0.025);
  mgA->GetYaxis()->SetTitleOffset(1.25);
  mgA->GetXaxis()->SetTitle("Volts");
  mgA->GetXaxis()->SetLabelSize(0.025);
  mgA->Draw("ap");

  c1->Update();
  TPaveStats *stats1 = (TPaveStats*)gr1->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats2 = (TPaveStats*)gr2->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats3 = (TPaveStats*)gr3->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats4 = (TPaveStats*)gr4->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats5 = (TPaveStats*)gr5->GetListOfFunctions()->FindObject("stats");
  stats1->SetTextColor(2);
  stats2->SetTextColor(3);
  stats3->SetTextColor(4);
  stats4->SetTextColor(6);
  stats5->SetTextColor(kOrange-3);
  stats1->SetX1NDC(0.34); stats1->SetX2NDC(0.54); stats1->SetY1NDC(0.75); stats1->SetY2NDC(0.82);
  stats2->SetX1NDC(0.34); stats2->SetX2NDC(0.54); stats2->SetY1NDC(0.82); stats2->SetY2NDC(0.89);
  stats3->SetX1NDC(0.13); stats3->SetX2NDC(0.33); stats3->SetY1NDC(0.68); stats3->SetY2NDC(0.75);
  stats4->SetX1NDC(0.13); stats4->SetX2NDC(0.33); stats4->SetY1NDC(0.75); stats4->SetY2NDC(0.82);
  stats5->SetX1NDC(0.13); stats5->SetX2NDC(0.33); stats5->SetY1NDC(0.82); stats5->SetY2NDC(0.89);

  c1->Modified();

  TLegend *legA = new TLegend(0.73, 0.89, 0.88, 0.73);
  legA->SetFillColor(0);
  legA->SetHeader("Legend");
  legA->AddEntry(gr5, "5 PE", "lp");
  legA->AddEntry(gr4, "4 PE", "lp");
  legA->AddEntry(gr3, "3 PE", "lp");
  legA->AddEntry(gr2, "2 PE", "lp");
  legA->AddEntry(gr1, "1 PE", "lp");
  legA->Draw();

  c1->Write("PhotoPeaks_vs_HV");
//  c1->Print("PhotoPeaks_vs_HV.pdf");





  c1->Clear();

  TMultiGraph *mgB = new TMultiGraph();
  TGraphErrors *grWidth1 = new TGraphErrors(NumEnt, hv, width1, 0, dwidth1);
  TGraphErrors *grWidth2 = new TGraphErrors(NumEnt, hv, width2, 0, dwidth2);
  TGraphErrors *grWidth3 = new TGraphErrors(NumEnt, hv, width3, 0, dwidth3);
  TGraphErrors *grWidth4 = new TGraphErrors(NumEnt, hv, width4, 0, dwidth4);
  TGraphErrors *grWidth5 = new TGraphErrors(NumEnt, hv, width5, 0, dwidth5);

  grWidth1->SetMarkerColor(2); grWidth1->SetTitle("1 PE"); grWidth1->SetMarkerStyle(20);;
//  grWidth1->Fit("f0","FS", "SAMES");
//  grWidth1->GetFunction("f0")->SetLineColor(1);
  mgB->Add(grWidth1);

  grWidth2->SetMarkerColor(3); grWidth2->SetTitle("2 PE"); grWidth2->SetMarkerStyle(20);
//  grWidth2->Fit("f0","FS", "SAMES");
//  grWidth2->GetFunction("f0")->SetLineColor(1);
  mgB->Add(grWidth2);

  grWidth3->SetMarkerColor(4); grWidth3->SetTitle("3 PE"); grWidth3->SetMarkerStyle(20);
//  grWidth3->Fit("f0","FS", "SAMES");
//  grWidth3->GetFunction("f0")->SetLineColor(1);
  mgB->Add(grWidth3);

  grWidth4->SetMarkerColor(6); grWidth4->SetTitle("4 PE"); grWidth4->SetMarkerStyle(20);
//  grWidth4->Fit("f0","FS", "SAMES");
//  grWidth4->GetFunction("f0")->SetLineColor(1);
  mgB->Add(grWidth4);

  grWidth5->SetMarkerColor(kOrange-3); grWidth5->SetTitle("5 PE"); grWidth5->SetMarkerStyle(20);
//  grWidth5->Fit("f0","FS", "SAMES");
//  grWidth5->GetFunction("f0")->SetLineColor(1);
  mgB->Add(grWidth5);
  mgB->Draw("ap");

  mgB->SetTitle("PE Peak Widths");
  mgB->SetMinimum(0);
  mgB->SetMaximum(500);
  mgB->GetYaxis()->SetTitle("Width in Integrated ADC Counts");
  mgB->GetYaxis()->SetLabelSize(0.025);
  mgB->GetYaxis()->SetTitleOffset(1.25);
  mgB->GetXaxis()->SetTitle("Volts");
  mgB->GetXaxis()->SetLabelSize(0.025);
  mgB->Draw("ap");
/*
  c1->Update();
  TPaveStats *statsWidth1 = (TPaveStats*)grWidth1->GetListOfFunctions()->FindObject("stats");
  TPaveStats *statsWidth2 = (TPaveStats*)grWidth2->GetListOfFunctions()->FindObject("stats");
  TPaveStats *statsWidth3 = (TPaveStats*)grWidth3->GetListOfFunctions()->FindObject("stats");
  TPaveStats *statsWidth4 = (TPaveStats*)grWidth4->GetListOfFunctions()->FindObject("stats");
  TPaveStats *statsWidth5 = (TPaveStats*)grWidth5->GetListOfFunctions()->FindObject("stats");
  statsWidth1->SetTextColor(2);
  statsWidth2->SetTextColor(3);
  statsWidth3->SetTextColor(4);
  statsWidth4->SetTextColor(6);
  statsWidth5->SetTextColor(kOrange-3);
  statsWidth1->SetX1NDC(0.34); statsWidth1->SetX2NDC(0.54); statsWidth1->SetY1NDC(0.75); statsWidth1->SetY2NDC(0.82);
  statsWidth2->SetX1NDC(0.34); statsWidth2->SetX2NDC(0.54); statsWidth2->SetY1NDC(0.82); statsWidth2->SetY2NDC(0.89);
  statsWidth3->SetX1NDC(0.13); statsWidth3->SetX2NDC(0.33); statsWidth3->SetY1NDC(0.68); statsWidth3->SetY2NDC(0.75);
  statsWidth4->SetX1NDC(0.13); statsWidth4->SetX2NDC(0.33); statsWidth4->SetY1NDC(0.75); statsWidth4->SetY2NDC(0.82);
  statsWidth5->SetX1NDC(0.13); statsWidth5->SetX2NDC(0.33); statsWidth5->SetY1NDC(0.82); statsWidth5->SetY2NDC(0.89);

  c1->Modified();
*/
  TLegend *legB = new TLegend(0.73, 0.89, 0.88, 0.73);
  legB->SetFillColor(0);
  legB->SetHeader("Legend");
  legB->AddEntry(grWidth5, "5 PE", "lp");
  legB->AddEntry(grWidth4, "4 PE", "lp");
  legB->AddEntry(grWidth3, "3 PE", "lp");
  legB->AddEntry(grWidth2, "2 PE", "lp");
  legB->AddEntry(grWidth1, "1 PE", "lp");
  legB->Draw();

//  c1->Print("PhotoPeakWidths_vs_HV.pdf");
  c1->Write("PhotoPeakWidths_vs_HV");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SLOPE VS NO. OF PE //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  c1->Clear();


  Double_t PEnum[5] = {1, 2, 3, 4, 5};

  TGraphErrors *slopeGr = new TGraphErrors(5, PEnum, slope, 0, dslope);

  slopeGr->SetMarkerColor(1); slopeGr->SetTitle("Slope vs. Num. of PE"); slopeGr->SetMarkerStyle(20);

  slopeGr->Fit("f0","FS", "SAMES");
  slopeGr->GetFunction("f0")->SetLineColor(1);
  double slpPerPEyInt = f0->GetParameter(0); double dslpPerPEyInt = f0->GetParError(0);
  double slpPerPEslpe = f0->GetParameter(1); double dslpPerPEslpe = f0->GetParError(1);


  slopeGr->Draw("ap");
  slopeGr->SetMinimum(0);
  slopeGr->SetMaximum(3000);
  slopeGr->GetYaxis()->SetTitle("Slope (fC/V)");
  slopeGr->GetYaxis()->SetTitleOffset(1.52);
  slopeGr->GetXaxis()->SetTitle("Number of PE");
  slopeGr->GetXaxis()->SetRangeUser(0.,6.); slopeGr->GetXaxis()->SetNdivisions(6);
  slopeGr->Draw("ap");


  c1->Update();
  TPaveStats *stsSlopeGr = (TPaveStats*)slopeGr->GetListOfFunctions()->FindObject("stats");
  stsSlopeGr->SetTextColor(1);
  stsSlopeGr->SetX1NDC(0.13); stsSlopeGr->SetX2NDC(0.33);
  stsSlopeGr->SetY1NDC(0.82); stsSlopeGr->SetY2NDC(0.89);
  c1->Modified();

//  c1->Print("Slope_vs_No_of_PE.pdf");
  c1->Write("Slope_vs_No_of_PE");

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// GAIN //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  c1->Clear();

  TGraphErrors *GainGr = new TGraphErrors(NumEnt, hv, Gain, 0, dGain);
  GainGr->Draw();
  GainGr->SetMarkerColor(1); GainGr->SetTitle("Gain vs. Reverse Voltage"); GainGr->SetMarkerStyle(8);
  GainGr->SetLineStyle(1);
  GainGr->Fit("f0","FS", "SAMES");
  GainGr->GetFunction("f0")->SetLineColor(1);


  GainGr->Draw("ap");
  GainGr->GetYaxis()->SetRangeUser(0, 8.0e6);
  GainGr->GetYaxis()->SetNdivisions(-1004);
  GainGr->GetYaxis()->SetTitle("#scale[1.0]{Gain} #scale[0.5]{#left(#frac{measured charge}{electron charge}#right)}");
  GainGr->GetYaxis()->SetLabelSize(0.025);
  GainGr->GetYaxis()->SetTitleOffset(1.33);
  GainGr->GetXaxis()->SetLabelSize(0.025);

  GainGr->Draw("ap");
  GainGr->GetXaxis()->SetTitle("Reverse Voltage (V)");
  c1->Update();
  GainGr->Draw("ap");


  c1->Update();
  TPaveStats *stsGainGr = (TPaveStats*)GainGr->GetListOfFunctions()->FindObject("stats");
  stsGainGr->SetTextColor(1);
  stsGainGr->SetX1NDC(0.54); stsGainGr->SetX2NDC(0.84);
  stsGainGr->SetY1NDC(0.21); stsGainGr->SetY2NDC(0.35);
  c1->Modified();

//  c1->Print("Gain_vs_No_of_PE.pdf");
  c1->Write("Gain_vs_No_of_PE");

////////////////////////////////////////////////////////////////////////////////
//////////////////////// PREDICTED PHOTOPEAK EVOLUTION /////////////////////////
////////////////////////////////////////////////////////////////////////////////

  c1->Clear();

  TMultiGraph *mgD = new TMultiGraph();

  stats1->SetX1NDC(1.3); stats1->SetX2NDC(1.5); mgD->Add(gr1);
  stats2->SetX1NDC(1.3); stats2->SetX2NDC(1.5); mgD->Add(gr2);
  stats3->SetX1NDC(1.3); stats3->SetX2NDC(1.5); mgD->Add(gr3);
  stats4->SetX1NDC(1.3); stats4->SetX2NDC(1.5); mgD->Add(gr4);
  stats5->SetX1NDC(1.3); stats5->SetX2NDC(1.5); mgD->Add(gr5);

  mgD->Draw("ap");

  mgD->SetTitle("Predicted Photo-Electron Peak Positions");
  mgD->SetMinimum(0);
  mgD->SetMaximum(20000);
  mgD->GetYaxis()->SetTitle("Mean Integrated ADC Counts");
  mgD->GetYaxis()->SetLabelSize(0.025);
  mgD->GetYaxis()->SetTitleOffset(1.45);
  mgD->GetXaxis()->SetLimits(70.0,71.3);
  mgD->GetXaxis()->SetTitle("Volts");
  mgD->GetXaxis()->SetLabelSize(0.025);

  mgD->Draw("ap");

  // Add lines to graph mgA for higher PE counts
  double grXmax = mgD->GetXaxis()->GetXmax();
  double grXmin = mgD->GetXaxis()->GetXmin();
  double lYmin, lYmax;
  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 10, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 10, x0avg, y0avg, grXmax);
  TLine *l_10PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_10PE->SetLineColor(2); l_10PE->SetLineWidth(3);

  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 20, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 20, x0avg, y0avg, grXmax);
  TLine *l_20PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_20PE->SetLineColor(3); l_20PE->SetLineWidth(3);

  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 30, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 30, x0avg, y0avg, grXmax);
  TLine *l_30PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_30PE->SetLineColor(4); l_30PE->SetLineWidth(3);

  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 40, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 40, x0avg, y0avg, grXmax);
  TLine *l_40PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_40PE->SetLineColor(5); l_40PE->SetLineWidth(3);

  Float_t HVsat40;
  HVsat40 = (SatPoint-lYmin)*(grXmax-grXmin)/(lYmax-lYmin) + grXmin;

  // draw exclusion zone
  TBox *yExcl = new TBox(grXmin, SatPoint, grXmax,20000);//"no border"
  yExcl->SetFillColor(2);
  yExcl->SetFillStyle(3003);
  yExcl->SetLineColor(2);
  yExcl->Draw();

  TPaveText* satReg = new TPaveText(.1, .82, .5, .88, "nbNDC");
  satReg->AddText("preamplifier saturation region");
  satReg->SetFillColor(2);
  satReg->SetFillStyle(3003);
  satReg->SetTextColor(6);
  satReg->Draw();

  l_10PE->Draw(); l_20PE->Draw(); l_30PE->Draw(); l_40PE->Draw();

  TLegend *legD = new TLegend(0.73, 0.89, 0.88, 0.73);
  legD->SetFillColor(0);
  legD->SetHeader("Legend");
  legD->AddEntry(l_40PE, "40 PE", "lp");
  legD->AddEntry(l_30PE, "30 PE", "lp");
  legD->AddEntry(l_20PE, "20 PE", "lp");
  legD->AddEntry(l_10PE, "10 PE", "lp");
  legD->AddEntry(gr5, "5 PE", "lp");
  legD->AddEntry(gr4, "4 PE", "lp");
  legD->AddEntry(gr3, "3 PE", "lp");
  legD->AddEntry(gr2, "2 PE", "lp");
  legD->AddEntry(gr1, "1 PE", "lp");

  legD->Draw();
//  c1->Print("PredictedPhotoPeaks_vs_HV.pdf");
  c1->Write("PredictedPhotoPeaks_vs_HV");

  return (HVsat40);
}
