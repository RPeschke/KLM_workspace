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
void PlotPhotoElectronPeaks_vs_HV() {
  gStyle->SetOptFit();
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  TFile* file = new TFile("PEpeaks.root", "READ");
  TTree* tree = (TTree*)file->Get("tree");

  Double_t Mean[5], Error[5]; Float_t HV;
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("Error", Error);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t peak1[NumEnt], peak2[NumEnt], peak3[NumEnt], peak4[NumEnt], peak5[NumEnt];
  Double_t dpeak1[NumEnt], dpeak2[NumEnt], dpeak3[NumEnt], dpeak4[NumEnt], dpeak5[NumEnt];

  Double_t p21[NumEnt], p32[NumEnt], p43[NumEnt], p54[NumEnt];
  Double_t dp21[NumEnt], dp32[NumEnt], dp43[NumEnt], dp54[NumEnt];

  Double_t p1Norm[NumEnt], p2Norm[NumEnt], p3Norm[NumEnt], p4Norm[NumEnt], p5Norm[NumEnt];
  Double_t dp1Norm[NumEnt], dp2Norm[NumEnt], dp3Norm[NumEnt], dp4Norm[NumEnt], dp5Norm[NumEnt];

  Double_t hv[NumEnt];

  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    peak1[i] = Mean[0]; dpeak1[i] = Error[0];
    peak2[i] = Mean[1]; dpeak2[i] = Error[1];
    peak3[i] = Mean[2]; dpeak3[i] = Error[2];
    peak4[i] = Mean[3]; dpeak4[i] = Error[3];
    peak5[i] = Mean[4]; dpeak5[i] = Error[4];
    p21[i] = peak2[i]-peak1[i];
    p32[i] = peak3[i]-peak2[i];
    p43[i] = peak4[i]-peak3[i];
    p54[i] = peak5[i]-peak4[i];
    dp21[i] = TMath::Sqrt(dpeak2[i]*dpeak2[i]+dpeak1[i]*dpeak1[i]);
    dp32[i] = TMath::Sqrt(dpeak3[i]*dpeak3[i]+dpeak2[i]*dpeak2[i]);
    dp43[i] = TMath::Sqrt(dpeak4[i]*dpeak4[i]+dpeak3[i]*dpeak3[i]);
    dp54[i] = TMath::Sqrt(dpeak5[i]*dpeak5[i]+dpeak4[i]*dpeak4[i]);
    p1Norm[i] = peak1[i]/1; dp1Norm[i] = dpeak1[i]/1;
    p2Norm[i] = peak2[i]/2; dp2Norm[i] = dpeak2[i]/2;
    p3Norm[i] = peak3[i]/3; dp3Norm[i] = dpeak3[i]/3;
    p4Norm[i] = peak4[i]/4; dp4Norm[i] = dpeak4[i]/4;
    p5Norm[i] = peak5[i]/5; dp5Norm[i] = dpeak5[i]/5;
    hv[i] = (Double_t)HV;
  }

  // fit functions
  TF1 *f0 = new TF1("f0", "pol1");

  TMultiGraph *mgA = new TMultiGraph();
  TGraphErrors *gr1 = new TGraphErrors(NumEnt, hv, peak1, 0, dpeak1);
  TGraphErrors *gr2 = new TGraphErrors(NumEnt, hv, peak2, 0, dpeak2);
  TGraphErrors *gr3 = new TGraphErrors(NumEnt, hv, peak3, 0, dpeak3);
  TGraphErrors *gr4 = new TGraphErrors(NumEnt, hv, peak4, 0, dpeak4);
  TGraphErrors *gr5 = new TGraphErrors(NumEnt, hv, peak5, 0, dpeak5);

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

  c1->Print("PhotoPeaks_vs_HV.pdf");
  delete file;
  delete c1;
}
////////////////////////////////////////////////////////////////////////////////
///////////////////////// DIFFERENCE BETWEEN EACH PEAK /////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DifferenceBetPeaks(){
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  TFile* file = new TFile("PEpeaks.root", "READ");
  TTree* tree = (TTree*)file->Get("tree");

  Double_t Mean[5], Error[5]; Float_t HV;
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("Error", Error);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t peak1[NumEnt], peak2[NumEnt], peak3[NumEnt], peak4[NumEnt], peak5[NumEnt];
  Double_t dpeak1[NumEnt], dpeak2[NumEnt], dpeak3[NumEnt], dpeak4[NumEnt], dpeak5[NumEnt];

  Double_t p21[NumEnt], p32[NumEnt], p43[NumEnt], p54[NumEnt];
  Double_t dp21[NumEnt], dp32[NumEnt], dp43[NumEnt], dp54[NumEnt];

  Double_t hv[NumEnt];


  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    peak1[i] = Mean[0]; dpeak1[i] = Error[0];
    peak2[i] = Mean[1]; dpeak2[i] = Error[1];
    peak3[i] = Mean[2]; dpeak3[i] = Error[2];
    peak4[i] = Mean[3]; dpeak4[i] = Error[3];
    peak5[i] = Mean[4]; dpeak5[i] = Error[4];
    p21[i] = peak2[i]-peak1[i];
    p32[i] = peak3[i]-peak2[i];
    p43[i] = peak4[i]-peak3[i];
    p54[i] = peak5[i]-peak4[i];
    dp21[i] = TMath::Sqrt(dpeak2[i]*dpeak2[i]+dpeak1[i]*dpeak1[i]);
    dp32[i] = TMath::Sqrt(dpeak3[i]*dpeak3[i]+dpeak2[i]*dpeak2[i]);
    dp43[i] = TMath::Sqrt(dpeak4[i]*dpeak4[i]+dpeak3[i]*dpeak3[i]);
    dp54[i] = TMath::Sqrt(dpeak5[i]*dpeak5[i]+dpeak4[i]*dpeak4[i]);
    hv[i] = (Double_t)HV;
  }

  TMultiGraph *mgB = new TMultiGraph();
  TGraphErrors *gr21 = new TGraphErrors(NumEnt, hv, p21, 0,dp21);
  TGraphErrors *gr32 = new TGraphErrors(NumEnt, hv, p32, 0,dp32);
  TGraphErrors *gr43 = new TGraphErrors(NumEnt, hv, p43, 0,dp43);
  TGraphErrors *gr54 = new TGraphErrors(NumEnt, hv, p54, 0,dp54);

  gr21->SetMarkerColor(2); gr21->SetTitle("2 PE - noise peak"); gr21->SetMarkerStyle(20);
  gr21->Fit("pol1","F"); gr21->GetFunction("pol1")->SetLineColor(1); mgB->Add(gr21);
  gr32->SetMarkerColor(3); gr32->SetTitle("3 PE - 1 PE"); gr32->SetMarkerStyle(20);
  gr32->Fit("pol1","F"); gr32->GetFunction("pol1")->SetLineColor(1); mgB->Add(gr32);
  gr43->SetMarkerColor(4); gr43->SetTitle("4 PE - 1 PE"); gr43->SetMarkerStyle(20);
  gr43->Fit("pol1","F"); gr43->GetFunction("pol1")->SetLineColor(1); mgB->Add(gr43);
  gr54->SetMarkerColor(6); gr54->SetTitle("5 PE - 1 PE"); gr54->SetMarkerStyle(20);
  gr54->Fit("pol1","F"); gr54->GetFunction("pol1")->SetLineColor(1); mgB->Add(gr54);

  mgB->Draw("ap");

  mgB->SetTitle("Photo-Electron Peaks and Noise Peak");
  mgB->SetMinimum(500);
  mgB->SetMaximum(1000);
  mgB->GetYaxis()->SetTitle("Difference in (Integrated) ADC Counts");
  mgB->GetYaxis()->SetTitleOffset(1.52);
  mgB->GetXaxis()->SetTitle("Volts");
  mgB->Draw("ap");
  c1->Update();

  TPaveStats *stats21 = (TPaveStats*)gr21->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats32 = (TPaveStats*)gr32->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats43 = (TPaveStats*)gr43->GetListOfFunctions()->FindObject("stats");
  TPaveStats *stats54 = (TPaveStats*)gr54->GetListOfFunctions()->FindObject("stats");
  stats21->SetTextColor(2);
  stats32->SetTextColor(3);
  stats43->SetTextColor(4);
  stats54->SetTextColor(6);
  stats21->SetX1NDC(0.34); stats21->SetX2NDC(0.54); stats21->SetY1NDC(0.75); stats21->SetY2NDC(0.82);
  stats32->SetX1NDC(0.34); stats32->SetX2NDC(0.54); stats32->SetY1NDC(0.82); stats32->SetY2NDC(0.89);
  stats43->SetX1NDC(0.13); stats43->SetX2NDC(0.33); stats43->SetY1NDC(0.75); stats43->SetY2NDC(0.82);
  stats54->SetX1NDC(0.13); stats54->SetX2NDC(0.33); stats54->SetY1NDC(0.82); stats54->SetY2NDC(0.89);
  c1->Modified();

  TLegend *legB = new TLegend(0.73, 0.89, 0.88, 0.73);
  legB->SetFillColor(0);
  legB->SetHeader("Legend");
  legB->AddEntry(gr54, "5 PE - 4 PE", "lp");
  legB->AddEntry(gr43, "4 PE - 3 PE", "lp");
  legB->AddEntry(gr32, "3 PE - 2 PE", "lp");
  legB->AddEntry(gr21, "2 PE - 1 PE", "lp");
  legB->Draw();

  c1->Print("PhotoPeakDifferences_vs_HV.pdf");

  delete file;
  delete c1;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////// NORMALIZED PER PE ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void NormalizedPhotoPeakEvolution(){
  gStyle->SetOptFit();
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  TFile* file = new TFile("PEpeaks.root", "READ");
  TTree* tree = (TTree*)file->Get("tree");

  Double_t Mean[5], Error[5]; Float_t HV;
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("Error", Error);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t peak1[NumEnt], peak2[NumEnt], peak3[NumEnt], peak4[NumEnt], peak5[NumEnt];
  Double_t dpeak1[NumEnt], dpeak2[NumEnt], dpeak3[NumEnt], dpeak4[NumEnt], dpeak5[NumEnt];

  Double_t p1Norm[NumEnt], p2Norm[NumEnt], p3Norm[NumEnt], p4Norm[NumEnt], p5Norm[NumEnt];
  Double_t dp1Norm[NumEnt], dp2Norm[NumEnt], dp3Norm[NumEnt], dp4Norm[NumEnt], dp5Norm[NumEnt];

  Double_t hv[NumEnt];

  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    peak1[i] = Mean[0]; dpeak1[i] = Error[0];
    peak2[i] = Mean[1]; dpeak2[i] = Error[1];
    peak3[i] = Mean[2]; dpeak3[i] = Error[2];
    peak4[i] = Mean[3]; dpeak4[i] = Error[3];
    peak5[i] = Mean[4]; dpeak5[i] = Error[4];
    p1Norm[i] = peak1[i]/1; dp1Norm[i] = dpeak1[i]/1;
    p2Norm[i] = peak2[i]/2; dp2Norm[i] = dpeak2[i]/2;
    p3Norm[i] = peak3[i]/3; dp3Norm[i] = dpeak3[i]/3;
    p4Norm[i] = peak4[i]/4; dp4Norm[i] = dpeak4[i]/4;
    p5Norm[i] = peak5[i]/5; dp5Norm[i] = dpeak5[i]/5;
    hv[i] = (Double_t)HV;
  }

  // fit functions
  TF1 *f0 = new TF1("f0", "pol1");

  TMultiGraph *mgC = new TMultiGraph();
  TGraphErrors *gr1Norm = new TGraphErrors(NumEnt, hv, p1Norm, 0, dp1Norm);
  TGraphErrors *gr2Norm = new TGraphErrors(NumEnt, hv, p2Norm, 0, dp2Norm);
  TGraphErrors *gr3Norm = new TGraphErrors(NumEnt, hv, p3Norm, 0, dp3Norm);
  TGraphErrors *gr4Norm = new TGraphErrors(NumEnt, hv, p4Norm, 0, dp4Norm);
  TGraphErrors *gr5Norm = new TGraphErrors(NumEnt, hv, p5Norm, 0, dp5Norm);

  double slopeNorm[5], dslopeNorm[5];

  gr1Norm->SetMarkerColor(2); gr1Norm->SetTitle("1 PE"); gr1Norm->SetMarkerStyle(20);
  gr1Norm->Fit("f0","FS", "SAMES"); slopeNorm[0] = f0->GetParameter(1); dslopeNorm[0] = f0->GetParError(1);
  gr1Norm->GetFunction("f0")->SetLineColor(1); mgC->Add(gr1Norm);

  gr2Norm->SetMarkerColor(3); gr2Norm->SetTitle("2 PE"); gr2Norm->SetMarkerStyle(20);
  gr2Norm->Fit("f0","FS", "SAMES"); slopeNorm[1] = f0->GetParameter(1); dslopeNorm[1] = f0->GetParError(1);
  gr2Norm->GetFunction("f0")->SetLineColor(1); mgC->Add(gr2Norm);

  gr3Norm->SetMarkerColor(4); gr3Norm->SetTitle("3 PE"); gr3Norm->SetMarkerStyle(20);
  gr3Norm->Fit("f0","FS", "SAMES"); slopeNorm[2] = f0->GetParameter(1); dslopeNorm[2] = f0->GetParError(1);
  gr3Norm->GetFunction("f0")->SetLineColor(1); mgC->Add(gr3Norm);

  gr4Norm->SetMarkerColor(6); gr4Norm->SetTitle("4 PE"); gr4Norm->SetMarkerStyle(20);
  gr4Norm->Fit("f0","FS", "SAMES"); slopeNorm[3] = f0->GetParameter(1); dslopeNorm[3] = f0->GetParError(1);
  gr4Norm->GetFunction("f0")->SetLineColor(1); mgC->Add(gr4Norm);

  gr5Norm->SetMarkerColor(kOrange-3); gr5Norm->SetTitle("5 PE"); gr5Norm->SetMarkerStyle(20);
  gr5Norm->Fit("f0","FS", "SAMES"); slopeNorm[4] = f0->GetParameter(1); dslopeNorm[4] = f0->GetParError(1);
  gr5Norm->GetFunction("f0")->SetLineColor(1); mgC->Add(gr5Norm);

  mgC->Draw("ap");

  mgC->SetTitle("Integrated Counts per PE vs. HV");
  mgC->SetMinimum(280);
  mgC->SetMaximum(1000);
  mgC->GetYaxis()->SetTitle("Integrated Wilkinson ADC Counts / PE");
  mgC->GetYaxis()->SetTitleOffset(1.52);
  mgC->GetXaxis()->SetTitle("Volts");
  mgC->Draw("ap");

  TPaveText *pt = new TPaveText(0.13, 0.61, 0.33, 0.66, "NDC");//"no border: Neutral Drawing Coordinates"
  char meanSlope[30];
  double mean = TMath::Mean(slopeNorm, slopeNorm+5);
  cout << mean << "\n";
  sprintf(meanSlope, "Mean Slope: %.3f", mean);
  cout << meanSlope << "\n";
  pt->AddText(meanSlope);
  pt->Draw();

   //draw a secondary x-axis// TGaxis(x1,y1,x2,y2,valLow, valHigh, Ndiv, opt)
  TGaxis *axis = new TGaxis(hv[0],340,hv[NumEnt-1],340,  166,185,210,"-");
  axis->SetLineColor(kGray+2);
  axis->SetTitle("8-bit trim-DAC setting"); axis->SetTitleColor(kGray+2);
  axis->SetTitleOffset(1.03); axis->SetTitleSize(0.02);
  axis->SetLabelSize(0.02); axis->SetLabelColor(kGray+2); axis->SetLabelOffset(-0.03);
  axis->Draw();

  c1->Update();
  TPaveStats *sts1Norm = (TPaveStats*)gr1Norm->GetListOfFunctions()->FindObject("stats");
  TPaveStats *sts2Norm = (TPaveStats*)gr2Norm->GetListOfFunctions()->FindObject("stats");
  TPaveStats *sts3Norm = (TPaveStats*)gr3Norm->GetListOfFunctions()->FindObject("stats");
  TPaveStats *sts4Norm = (TPaveStats*)gr4Norm->GetListOfFunctions()->FindObject("stats");
  TPaveStats *sts5Norm = (TPaveStats*)gr5Norm->GetListOfFunctions()->FindObject("stats");
  sts1Norm->SetTextColor(2);
  sts2Norm->SetTextColor(3);
  sts3Norm->SetTextColor(4);
  sts4Norm->SetTextColor(6);
  sts5Norm->SetTextColor(kOrange-3);
  sts1Norm->SetX1NDC(0.34); sts1Norm->SetX2NDC(0.54); sts1Norm->SetY1NDC(0.75); sts1Norm->SetY2NDC(0.82);
  sts2Norm->SetX1NDC(0.34); sts2Norm->SetX2NDC(0.54); sts2Norm->SetY1NDC(0.82); sts2Norm->SetY2NDC(0.89);
  sts3Norm->SetX1NDC(0.13); sts3Norm->SetX2NDC(0.33); sts3Norm->SetY1NDC(0.68); sts3Norm->SetY2NDC(0.75);
  sts4Norm->SetX1NDC(0.13); sts4Norm->SetX2NDC(0.33); sts4Norm->SetY1NDC(0.75); sts4Norm->SetY2NDC(0.82);
  sts5Norm->SetX1NDC(0.13); sts5Norm->SetX2NDC(0.33); sts5Norm->SetY1NDC(0.82); sts5Norm->SetY2NDC(0.89);
  c1->Modified();

  TLegend *legC = new TLegend(0.73, 0.89, 0.88, 0.73);
  legC->SetFillColor(0);
  legC->SetHeader("Legend");
  legC->AddEntry(gr5Norm, "5 PE", "lp");
  legC->AddEntry(gr4Norm, "4 PE", "lp");
  legC->AddEntry(gr3Norm, "3 PE", "lp");
  legC->AddEntry(gr2Norm, "2 PE", "lp");
  legC->AddEntry(gr1Norm, "1 PE", "lp");
  legC->Draw();

  c1->Print("NormalizedPhotoPeakEvolution.pdf");

  delete file;
  delete c1;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SLOPE VS NO. OF PE //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void PhotoPeakEvolutionVsNoPE(){
  gStyle->SetOptFit();
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  TFile* file = new TFile("PEpeaks.root", "READ");
  TTree* tree = (TTree*)file->Get("tree");

  Double_t Mean[5], Error[5]; Float_t HV;
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("Error", Error);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t peak1[NumEnt], peak2[NumEnt], peak3[NumEnt], peak4[NumEnt], peak5[NumEnt];
  Double_t dpeak1[NumEnt], dpeak2[NumEnt], dpeak3[NumEnt], dpeak4[NumEnt], dpeak5[NumEnt];

  Double_t p21[NumEnt], p32[NumEnt], p43[NumEnt], p54[NumEnt];
  Double_t dp21[NumEnt], dp32[NumEnt], dp43[NumEnt], dp54[NumEnt];

  Double_t p1Norm[NumEnt], p2Norm[NumEnt], p3Norm[NumEnt], p4Norm[NumEnt], p5Norm[NumEnt];
  Double_t dp1Norm[NumEnt], dp2Norm[NumEnt], dp3Norm[NumEnt], dp4Norm[NumEnt], dp5Norm[NumEnt];

  Double_t hv[NumEnt];

  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    peak1[i] = Mean[0]; dpeak1[i] = Error[0];
    peak2[i] = Mean[1]; dpeak2[i] = Error[1];
    peak3[i] = Mean[2]; dpeak3[i] = Error[2];
    peak4[i] = Mean[3]; dpeak4[i] = Error[3];
    peak5[i] = Mean[4]; dpeak5[i] = Error[4];
    p21[i] = peak2[i]-peak1[i];
    p32[i] = peak3[i]-peak2[i];
    p43[i] = peak4[i]-peak3[i];
    p54[i] = peak5[i]-peak4[i];
    dp21[i] = TMath::Sqrt(dpeak2[i]*dpeak2[i]+dpeak1[i]*dpeak1[i]);
    dp32[i] = TMath::Sqrt(dpeak3[i]*dpeak3[i]+dpeak2[i]*dpeak2[i]);
    dp43[i] = TMath::Sqrt(dpeak4[i]*dpeak4[i]+dpeak3[i]*dpeak3[i]);
    dp54[i] = TMath::Sqrt(dpeak5[i]*dpeak5[i]+dpeak4[i]*dpeak4[i]);
    p1Norm[i] = peak1[i]/1; dp1Norm[i] = dpeak1[i]/1;
    p2Norm[i] = peak2[i]/2; dp2Norm[i] = dpeak2[i]/2;
    p3Norm[i] = peak3[i]/3; dp3Norm[i] = dpeak3[i]/3;
    p4Norm[i] = peak4[i]/4; dp4Norm[i] = dpeak4[i]/4;
    p5Norm[i] = peak5[i]/5; dp5Norm[i] = dpeak5[i]/5;
    hv[i] = (Double_t)HV;
  }

  // fit functions
  TF1 *f0 = new TF1("f0", "pol1");

  Double_t PEnum[5] = {1, 2, 3, 4, 5};

  TGraphErrors *slopeGr = new TGraphErrors(5, PEnum, slope, 0, dslope);

  slopeGr->SetMarkerColor(1); slopeGr->SetTitle("slope vs. No. of PE"); slopeGr->SetMarkerStyle(20);
  slopeGr->Fit("f0","FS", "SAMES");
  slopeGr->GetFunction("f0")->SetLineColor(1);
  double slpPerPEyInt = f0->GetParameter(0); double dslpPerPEyInt = f0->GetParError(0);
  double slpPerPEslpe = f0->GetParameter(1); double dslpPerPEslpe = f0->GetParError(1);


  slopeGr->Draw("ap");

  slopeGr->SetTitle("Slope vs No. of PE");
  slopeGr->SetMinimum(0);
  slopeGr->SetMaximum(3000);
  slopeGr->GetYaxis()->SetTitle("Average Integrated Wilkinson ADC Counts / Volt");
  slopeGr->GetYaxis()->SetTitleOffset(1.52);
  slopeGr->GetXaxis()->SetTitle("No. of Photo Electrons");
  slopeGr->GetXaxis()->SetRangeUser(0.,6.); slopeGr->GetXaxis()->SetNdivisions(6);
  slopeGr->Draw("ap");


  c1->Update();
  TPaveStats *stsSlopeGr = (TPaveStats*)slopeGr->GetListOfFunctions()->FindObject("stats");
  stsSlopeGr->SetTextColor(1);
  stsSlopeGr->SetX1NDC(0.13); stsSlopeGr->SetX2NDC(0.33);
  stsSlopeGr->SetY1NDC(0.82); stsSlopeGr->SetY2NDC(0.89);
  c1->Modified();

  c1->Print("Slope_vs_No_of_PE.pdf");
}
////////////////////////////////////////////////////////////////////////////////
//////////////////////// PREDICTED PHOTOPEAK EVOLUTION /////////////////////////
////////////////////////////////////////////////////////////////////////////////
void PredictedPhotoPeakEvolution(){
  gStyle->SetOptFit();
  TCanvas *c1 = new TCanvas("c1","A Simple Graph with error bars", 1000, 1000);
  //c1->SetFillColor(43);
  c1->SetGrid();
  //c1->GetFrame()->SetFillColor(32);
  c1->GetFrame()->SetBorderSize(12);

  TFile* file = new TFile("PEpeaks.root", "READ");
  TTree* tree = (TTree*)file->Get("tree");

  Double_t Mean[5], Error[5]; Float_t HV;
  tree->SetBranchAddress("Mean", Mean);
  tree->SetBranchAddress("Error", Error);
  tree->SetBranchAddress("HV", &HV);

  // declare plot variables
  int NumEnt = tree->GetEntriesFast();
  Double_t peak1[NumEnt], peak2[NumEnt], peak3[NumEnt], peak4[NumEnt], peak5[NumEnt];
  Double_t dpeak1[NumEnt], dpeak2[NumEnt], dpeak3[NumEnt], dpeak4[NumEnt], dpeak5[NumEnt];

  Double_t p21[NumEnt], p32[NumEnt], p43[NumEnt], p54[NumEnt];
  Double_t dp21[NumEnt], dp32[NumEnt], dp43[NumEnt], dp54[NumEnt];

  Double_t p1Norm[NumEnt], p2Norm[NumEnt], p3Norm[NumEnt], p4Norm[NumEnt], p5Norm[NumEnt];
  Double_t dp1Norm[NumEnt], dp2Norm[NumEnt], dp3Norm[NumEnt], dp4Norm[NumEnt], dp5Norm[NumEnt];

  Double_t hv[NumEnt];

  // fill plot variables
  for (int i=0; i<NumEnt; i++){
    tree->GetEntry(i);
    peak1[i] = Mean[0]; dpeak1[i] = Error[0];
    peak2[i] = Mean[1]; dpeak2[i] = Error[1];
    peak3[i] = Mean[2]; dpeak3[i] = Error[2];
    peak4[i] = Mean[3]; dpeak4[i] = Error[3];
    peak5[i] = Mean[4]; dpeak5[i] = Error[4];
    p21[i] = peak2[i]-peak1[i];
    p32[i] = peak3[i]-peak2[i];
    p43[i] = peak4[i]-peak3[i];
    p54[i] = peak5[i]-peak4[i];
    dp21[i] = TMath::Sqrt(dpeak2[i]*dpeak2[i]+dpeak1[i]*dpeak1[i]);
    dp32[i] = TMath::Sqrt(dpeak3[i]*dpeak3[i]+dpeak2[i]*dpeak2[i]);
    dp43[i] = TMath::Sqrt(dpeak4[i]*dpeak4[i]+dpeak3[i]*dpeak3[i]);
    dp54[i] = TMath::Sqrt(dpeak5[i]*dpeak5[i]+dpeak4[i]*dpeak4[i]);
    p1Norm[i] = peak1[i]/1; dp1Norm[i] = dpeak1[i]/1;
    p2Norm[i] = peak2[i]/2; dp2Norm[i] = dpeak2[i]/2;
    p3Norm[i] = peak3[i]/3; dp3Norm[i] = dpeak3[i]/3;
    p4Norm[i] = peak4[i]/4; dp4Norm[i] = dpeak4[i]/4;
    p5Norm[i] = peak5[i]/5; dp5Norm[i] = dpeak5[i]/5;
    hv[i] = (Double_t)HV;
  }


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
  l_20PE->SetLineColor(3); l_20PE->SetLineWi  c1->Clear();dth(3);

  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 30, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 30, x0avg, y0avg, grXmax);
  TLine *l_30PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_30PE->SetLineColor(4); l_30PE->SetLineWidth(3);

  lYmin = YofPE(slpPerPEslpe, slpPerPEyInt, 40, x0avg, y0avg, grXmin);
  lYmax = YofPE(slpPerPEslpe, slpPerPEyInt, 40, x0avg, y0avg, grXmax);
  TLine *l_40PE = new TLine(grXmin, lYmin, grXmax, lYmax);
  l_40PE->SetLineColor(5); l_40PE->SetLineWidth(3);



  // draw exclusion zone
  TBox *yExcl = new TBox(grXmin, 17500, grXmax,20000);//"no border"
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

  c1->Print("PredictedPhotoPeaks_vs_HV.pdf");
}
