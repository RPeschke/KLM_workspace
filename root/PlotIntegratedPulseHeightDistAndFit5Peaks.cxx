void PlotIntegratedPulseDistAndFit5Peaks(const char* root_file, const char* plotTitle, const float actHV, const int chNo) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);

  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t PeakTime[16];
  Float_t RawSample[32], PeakVal[16], ToverTh[16];
  Float_t Sample[16][128], PedSample[16][512][32];
  Float_t PartialRiemannSum[16], RiemannSum[16], RiemannSumSq[16];
  Float_t AbsValue[16];


  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("WrAddNum", &WrAddNum);
  tree->SetBranchAddress("Wctime", &Wctime);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime", PeakTime);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("PartialRiemannSum", PartialRiemannSum);
  tree->SetBranchAddress("RiemannSum", RiemannSum);
  tree->SetBranchAddress("RiemannSumSq", RiemannSumSq);
  tree->SetBranchAddress("AbsValue", AbsValue);
  tree->SetBranchAddress("TimeOverThresh", ToverTh);
  tree->SetBranchAddress("ADC_counts", Sample);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->Divide(2,2);


  // Declare memory on heap for histogram & configure
  TH1F* hist0 = new TH1F("", ";Peak Value Distribution;", 300,0,800);
  hist0->SetTitle(plotTitle);
  hist0->SetLineColor(13);

  TH1F* hist1 = new TH1F("", ";Riemann Sum;", 300,-500,5500);
  hist1->SetTitle(plotTitle);
  hist1->SetLineColor(13);

  TH1F* hist2 = new TH1F("", ";Partial Riemann Sum;", 300,-2000,6000);
  hist2->SetTitle(plotTitle);
  hist2->SetLineColor(13);

  TH1F* hist3 = new TH1F("", ";Sum of Squares;", 300,0,6000000);
  hist3->SetTitle(plotTitle);
  hist3->SetLineColor(13);

  TH1F* hist4 = new TH1F("", ";Sum of Absolute Values;", 300, 0, 8000);
  hist4->SetTitle(plotTitle);
  hist4->SetLineColor(13);

  TH1F* hist5 = new TH1F("", ";Time Over 1/3 Peak Value;", 128,0,128);
  hist5->SetTitle(plotTitle);
  hist5->SetLineColor(13);

  TH1F* hist6 = new TH1F("", ";Time Over 1/3 Peak, times Peak;", 400, 0, 102400);
  hist6->SetTitle(plotTitle);
  hist6->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    hist0->Fill(PeakVal[chNo]);
    hist1->Fill(RiemannSum[chNo]);
    hist2->Fill(PartialRiemannSum[chNo]);
    hist3->Fill(RiemannSumSq[chNo]);
    hist4->Fill(AbsValue[chNo]);
    hist5->Fill(ToverTh[chNo]);
    hist6->Fill(ToverTh[chNo]*PeakVal[chNo]);
  }

  canv->cd(1); gPad->SetLogy();
  hist0->Draw();

  canv->cd(2); gPad->SetLogy();
  hist1->Draw();

  canv->cd(3); gPad->SetLogy();
  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)(hist2->Clone());
  TH1F* clone2 = (TH1F*)(hist2->Clone());
  TH1F* clone3 = (TH1F*)(hist2->Clone());
  TH1F* clone4 = (TH1F*)(hist2->Clone());

  float width = 250.;
  Double_t Mean[5], Error[5];
  Float_t HV = actHV;
  float expNP  = HV*4.81853e+02-3.37660e+04;
  float exp1PE = HV*9.78835e+02-6.84374e+04;
  float exp2PE = HV*1.54434e+03-1.07921e+05;
  float exp3PE = HV*2.12429e+03-1.48415e+05;
  float exp4PE = HV*2.46639e+03-1.71947e+05;

  TF1 *f0 = new TF1("f0", "gaus", (expNP-width), (expNP+width));
  f0->SetLineColor(2);
  hist2->Fit("f0", "RS"); // "R" for fit range
  Mean[0] = f0->GetParameter(1); Error[0] = f0->GetParError(1);
  hist2->Draw();

  TF1 *f1 = new TF1("f1", "gaus", (exp1PE-width), (exp1PE+width));
  f1->SetLineColor(3);
  clone1->Fit("f1", "RS", "SAMES");
  Mean[1] = f1->GetParameter(1); Error[1] = f1->GetParError(1);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

  TF1 *f2 = new TF1("f2", "gaus", (exp2PE-width), (exp2PE+width));
  f2->SetLineColor(4);
  clone2->Fit("f2", "RS", "SAMES");
  Mean[2] = f2->GetParameter(1); Error[2] = f2->GetParError(1);
  clone2->Draw("SAMES");

  TF1 *f3 = new TF1("f3", "gaus", (exp3PE-width), (exp3PE+width));
  f3->SetLineColor(6);
  clone3->Fit("f3", "RS", "SAMES");
  Mean[3] = f3->GetParameter(1); Error[3] = f3->GetParError(1);
  clone3->Draw("SAMES");

  TF1 *f4 = new TF1("f4", "gaus", (exp4PE-width), (exp4PE+width));
  f4->SetLineColor(38);
  clone4->Fit("f4", "RS", "SAMES");
  Mean[4] = f4->GetParameter(1); Error[4] = f4->GetParError(1);
  clone4->Draw("SAMES");

  // Modify Stat boxes to include ALL fit results
  gPad->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
  TPaveStats *stat0 = (TPaveStats*)(hist2->FindObject("stats"));
  TPaveStats *stat1 = (TPaveStats*)(clone1->FindObject("stats"));
  TPaveStats *stat2 = (TPaveStats*)(clone2->FindObject("stats"));
  TPaveStats *stat3 = (TPaveStats*)(clone3->FindObject("stats"));
  TPaveStats *stat4 = (TPaveStats*)(clone4->FindObject("stats"));
  if(stat0 && stat1 && stat2 && stat3 && stat4) {
    stat0->SetTextColor(2);
    stat0->Draw();

    stat1->SetTextColor(3);
    stat1->SetY1NDC(stat0->GetY1NDC() - height);
    stat1->SetY2NDC(stat0->GetY1NDC() );
    stat1->Draw();

    stat2->SetTextColor(4);
    stat2->SetY1NDC(stat1->GetY1NDC() - height);
    stat2->SetY2NDC(stat1->GetY1NDC());
    stat2->Draw();

    stat3->SetTextColor(6);
    stat3->SetY1NDC(stat2->GetY1NDC() - height);
    stat3->SetY2NDC(stat2->GetY1NDC());
    stat3->Draw();

    stat4->SetTextColor(38);
    stat4->SetY1NDC(stat3->GetY1NDC() - height);
    stat4->SetY2NDC(stat3->GetY1NDC());
    stat4->Draw();
  }
  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf(", plotTitle);
  canv->Print(pdfOutfile);

  canv->cd(4); gPad->SetLogy();
  hist3->Draw();

  canv->cd(1); gPad->SetLogy();
  hist3->Draw();

  canv->cd(2); gPad->SetLogy();
  hist3->Draw();

  sprintf(pdfOutfile, "%s.pdf)", plotTitle);
  canv->Print(pdfOutfile);

  //write fit results to new TTree
  TFile* results = new TFile("PEpeaks.root", "UPDATE");
  TTree* tree1;
  if (results->Get("tree")){
    tree1 = (TTree*)results->Get("tree");
    tree1->SetBranchAddress("Mean", Mean);
    tree1->SetBranchAddress("Error", Error);
    tree1->SetBranchAddress("HV", &HV);
  }
  else {
    tree1 = new TTree("tree", "Single PE Gaussian fit results for SiPM pulse distributions");
    tree1->Branch("Mean", Mean, "Mean[5]/D");
    tree1->Branch("Error", Error, "Error[5]/D");
    tree1->Branch("HV", &HV, "HV/F");
  }
  tree1->Fill();
  tree1->Write("tree", TObject::kWriteDelete);

  delete canv;
  delete results;
  delete file;
}
