void PlotSingleChannelPulseAnalysis(const char* root_file, int chNo, int mVp) {
  gROOT->Reset();
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size

  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];
  Float_t PartialRiemannSum[16], RiemannSum[16], RiemannSumSq[16];
  Float_t SampNum[128], ChSample[128], ToverTh[16];

  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canvas1->Divide(2, 2); // make 4 pads per canvas

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
  tree->SetBranchAddress("TimeOverThresh", ToverTh);
  tree->SetBranchAddress("ADC_counts", Sample);



  TMultiGraph* mg = new TMultiGraph();
  for (int i=0; i<128; i++) SampNum[i] = (float)i;

  tree->GetEntry(0); for(int j=0; j<128; j++) ChSample[j] = Sample[chNo][j];
  TGraph* gr1 = new TGraph(128, SampNum, ChSample);
  gr1->SetMarkerColor(1); gr1->SetTitle("1"); gr1->SetMarkerStyle(20); mg->Add(gr1);

  tree->GetEntry(1); for(int j=0; j<128; j++) ChSample[j] = Sample[chNo][j];
  TGraph* gr2 = new TGraph(128, SampNum, ChSample);
  gr2->SetMarkerColorAlpha(2, .3); gr2->SetTitle("2"); gr2->SetMarkerStyle(7); mg->Add(gr2);

  tree->GetEntry(2); for(int j=0; j<128; j++) ChSample[j] = Sample[chNo][j];
  TGraph* gr3 = new TGraph(128, SampNum, ChSample);
  gr3->SetMarkerColorAlpha(3, .3); gr3->SetTitle("3"); gr3->SetMarkerStyle(7); mg->Add(gr3);

  tree->GetEntry(3); for(int j=0; j<128; j++) ChSample[j] = Sample[chNo][j];
  TGraph* gr4 = new TGraph(128, SampNum, ChSample);
  gr4->SetMarkerColorAlpha(4, .3); gr4->SetTitle("4"); gr4->SetMarkerStyle(7); mg->Add(gr4);


  canvas1->cd(2);

  mg->Draw("ap");

  char xlabel[18];
  sprintf(xlabel, "Ch_%d Sample No.", chNo);
  mg->SetTitle("First 4 Events");
  mg->GetXaxis()->SetTitle(xlabel);
  mg->GetYaxis()->SetTitleOffset(1.5);
  mg->GetYaxis()->SetTitle("ADC Counts");
  mg->SetMaximum(700);
  mg->SetMinimum(-300);

  TF1 *f0 = new TF1("f0", "gaus");

  TH1F* h2 = new TH1F("", "Riemann Sum", 300, -13000, 2000);
  TH1F* h3 = new TH1F("", "Sum of Squares", 300, 0, 10000000);
  TH1F* h4 = new TH1F("", "Partial Riemann Sum", 300, 0, 6000);
  TH1I* h5 = new TH1I("","Window Count Between Triggers", 512, 0, 511);
  TH1F* h6 = new TH1F("","Peak Value", 300, 0, 600);
  TH1F* h7 = new TH1F("","Time over 1/3", 128, 0, 128);


  int numEnt = tree->GetEntriesFast();
  int winNum[numEnt], deltaWin;
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    h2->Fill(RiemannSum[chNo]);
    h3->Fill(RiemannSumSq[chNo]);
    h4->Fill(PartialRiemannSum[chNo]);
    h6->Fill(PeakVal[chNo]);
    h7->Fill(ToverTh[chNo]);
    winNum[e] = AddNum;
  }

  Double_t AvgSum[2], AvgSumSigma[2];
  Double_t AvgSumSq[2], AvgSumSqSigma[2];
  Double_t AvgBoundSum[2],  AvgBoundSumSigma[2];
  Double_t AvgPeak[2], AvgPeakSigma[2];
  Double_t AvgToT[2], AvgToTSigma[2];

  canvas1->cd(3); h2->Draw();
  h2->Fit("f0", "FS", "SAMES");
  AvgSum[0]  = f0->GetParameter(1); AvgSum[1]  = f0->GetParError(1);
  AvgSumSigma[0] = f0->GetParameter(2); AvgSumSigma[1] = f0->GetParError(2);
  h2->GetFunction("f0")->SetLineColorAlpha(13,.7);

  canvas1->cd(4); h3->Draw();
  h3->Fit("f0", "FS", "SAMES");
  AvgSumSq[0] = f0->GetParameter(1); AvgSumSq[1]  = f0->GetParError(1);
  AvgSumSqSigma[0] = f0->GetParameter(2); AvgSumSqSigma[1] = f0->GetParError(2);
  h3->GetFunction("f0")->SetLineColorAlpha(13,.7);


  canvas1->Update();
  canvas1->Print("test.pdf(");
  canvas1->Clear();
  canvas1->Divide(2,2);

  canvas1->cd(1); h4->Draw();
  h4->Fit("f0", "FS", "SAMES");
  AvgBoundSum[0]  = f0->GetParameter(1); AvgBoundSum[1]  = f0->GetParError(1);
  AvgBoundSumSigma[0] = f0->GetParameter(2); AvgBoundSumSigma[1] = f0->GetParError(2);
  h4->GetFunction("f0")->SetLineColorAlpha(13,.7);

  for(int e=1; e<numEnt; e++) {
    deltaWin = (winNum[e]-winNum[e-1]+512)%512;
    h5->Fill(deltaWin);
  }
  canvas1->cd(2); h5->Draw(); h5->SetMaximum(numEnt);
  h5->Fit("f0", "FS", "SAMES");
  h5->GetFunction("f0")->SetLineColorAlpha(13,.7);

  canvas1->cd(3); h6->Draw();
  h6->Fit("f0", "FS", "SAMES");
  AvgPeak[0]  = f0->GetParameter(1); AvgPeak[1]  = f0->GetParError(1);
  AvgPeakSigma[0] = f0->GetParameter(2); AvgPeakSigma[1] = f0->GetParError(2);
  h6->GetFunction("f0")->SetLineColorAlpha(13,.7);

  canvas1->cd(4); h7->Draw();
  h7->Fit("f0", "FS", "SAMES");
  AvgToT[0]  = f0->GetParameter(1); AvgToT[1]  = f0->GetParError(1);
  AvgToTSigma[0] = f0->GetParameter(2); AvgToTSigma[1] = f0->GetParError(2);
  h7->GetFunction("f0")->SetLineColorAlpha(13,.7);

  canvas1->Update();
  canvas1->Print("test.pdf)");

  // Write fit results to new TTree
  TFile* results = new TFile("PulseAnalysis.root", "UPDATE");
  TTree* tree1;
  if (results->Get("tree")){
    tree1 = (TTree*)results->Get("tree");
    tree1->SetBranchAddress("Channel", &chNo);
    tree1->SetBranchAddress("VpPulse_mV", &mVp);
    tree1->SetBranchAddress("AvgSum", &AvgSum);
    tree1->SetBranchAddress("AvgSumSq", &AvgSumSq);
    tree1->SetBranchAddress("AvgBoundSum", &AvgBoundSum);
    tree1->SetBranchAddress("AvgPeak", &AvgPeak);
    tree1->SetBranchAddress("AvgToT", &AvgToT);
    tree1->SetBranchAddress("AvgSumSigma", &AvgSumSigma);
    tree1->SetBranchAddress("AvgSumSqSigma", &AvgSumSqSigma);
    tree1->SetBranchAddress("AvgBoundSumSigma", &AvgBoundSumSigma);
    tree1->SetBranchAddress("AvgPeakSigma", &AvgPeakSigma);
    tree1->SetBranchAddress("AvgToTSigma", &AvgToTSigma);
  }
  else {
    tree1 = new TTree("tree", "0==fitted value, 1==fit error");
    tree1->Branch("Channel", &chNo, "chNo/I");
    tree1->Branch("VpPulse_mV", &mVp, "VpPulse_mV/I");
    tree1->Branch("AvgSum", &AvgSum, "AvgSum[2]/D");
    tree1->Branch("AvgSumSq", &AvgSumSq,"AvgSumSq[2]/D");
    tree1->Branch("AvgBoundSum", &AvgBoundSum, "AvgBoundSum[2]/D");
    tree1->Branch("AvgPeak", &AvgPeak, "AvgPeak[2]/D");
    tree1->Branch("AvgToT", &AvgToT, "AvgToT[2]/D");
    tree1->Branch("AvgSumSigma", &AvgSumSigma, "AvgSumSigma[2]/D");
    tree1->Branch("AvgSumSqSigma", &AvgSumSqSigma,"AvgSumSqSigma[2]/D");
    tree1->Branch("AvgBoundSumSigma", &AvgBoundSumSigma, "AvgBoundSumSigma[2]/D");
    tree1->Branch("AvgPeakSigma", &AvgPeakSigma, "AvgPeakSigma[2]/D");
    tree1->Branch("AvgToTSigma", &AvgToTSigma, "AvgToTSigma[2]/D");
  }
  tree1->Fill();
  tree1->Write("tree", TObject::kWriteDelete);
  results->Close();
  file->Close();
  delete canvas1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SingleChannelPulseAnalysisSummaryPlots(){

  Int_t    chNo, mVpk;
  Double_t AvgSum[2], AvgSumSigma[2];
  Double_t AvgSumSq[2], AvgSumSqSigma[2];
  Double_t AvgBoundSum[2],  AvgBoundSumSigma[2];
  Double_t AvgPeak[2], AvgPeakSigma[2];
  Double_t AvgToT[2], AvgToTSigma[2];

  TFile* infile = new TFile("PulseAnalysis.root", "READ");
  TTree* tree2;
  if (infile->Get("tree")){
    tree2 = (TTree*)infile->Get("tree");
    tree2->SetBranchAddress("Channel", &chNo);
    tree2->SetBranchAddress("VpPulse_mV", &mVpk);
    tree2->SetBranchAddress("AvgSum", &AvgSum);
    tree2->SetBranchAddress("AvgSumSq", &AvgSumSq);
    tree2->SetBranchAddress("AvgBoundSum", &AvgBoundSum);
    tree2->SetBranchAddress("AvgPeak", &AvgPeak);
    tree2->SetBranchAddress("AvgToT", &AvgToT);
    tree2->SetBranchAddress("AvgSumSigma", &AvgSumSigma);
    tree2->SetBranchAddress("AvgSumSqSigma", &AvgSumSqSigma);
    tree2->SetBranchAddress("AvgBoundSumSigma", &AvgBoundSumSigma);
    tree2->SetBranchAddress("AvgPeakSigma", &AvgPeakSigma);
    tree2->SetBranchAddress("AvgToTSigma", &AvgToTSigma);
  }
  TCanvas* canv = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canv->Divide(2, 2); // make 4 pads per canvas

  //TMultiGraph* mg2 = new TMultiGraph();

  int NoEnt = tree2->GetEntriesFast();
  Double_t Sum[NoEnt], SumErr[NoEnt], SumSigma[NoEnt], SumSigmaErr[NoEnt];
  Double_t SumSq[NoEnt], SumSqErr[NoEnt], SumSqSigma[NoEnt],SumSqSigmaErr[NoEnt];
  Double_t BoundSum[NoEnt],  BoundSumErr[NoEnt], BoundSumSigma[NoEnt], BoundSumSigmaErr[NoEnt];
  Double_t Peak[NoEnt], PeakErr[NoEnt],PeakSigma[NoEnt], PeakSigmaErr[NoEnt];
  Double_t ToT[NoEnt], ToTErr[NoEnt], ToTSigma[NoEnt], ToTSigmaErr[NoEnt];
  Double_t mVpeak[NoEnt];
  for (int e=0; e<NoEnt; e++) {
    tree2->GetEntry(e);
    Sum[e]           = AvgSum[0];           SumErr[e]           = AvgSum[1];
    SumSigma[e]      = AvgSumSigma[0];      SumSigmaErr[e]      = AvgSumSigma[1];
    SumSq[e]         = AvgSumSq[0];         SumSqErr[e]         = AvgSumSq[1];
    SumSqSigma[e]    = AvgSumSigma[0];      SumSqSigmaErr[e]    = AvgSumSigma[1];
    BoundSum[e]      = AvgBoundSum[0];      BoundSumErr[e]      = AvgBoundSum[1];
    BoundSumSigma[e] = AvgBoundSumSigma[0]; BoundSumSigmaErr[e] = AvgBoundSumSigma[1];
    Peak[e]          = AvgPeak[0];          PeakErr[e]          = AvgPeak[1];
    PeakSigma[e]     = AvgPeakSigma[0];     PeakSigmaErr[e]     = AvgPeakSigma[1];
    ToT[e]           = AvgToT[0];           ToTErr[e]           = AvgToT[1];
    ToTSigma[e]      = AvgToTSigma[0];      ToTSigmaErr[e]      = AvgToTSigma[1];
    mVpeak[e] = mVpk;
  }
  canv->cd(1);
  TGraphErrors* ge1 = new TGraphErrors(NoEnt, mVpeak, Sum, 0, SumErr);
  ge1->SetTitle("Rieman Sum"); ge1->SetMarkerStyle(20); ge1->Draw("ap");
  canv->cd(2);

  TGraphErrors* ge2 = new TGraphErrors(NoEnt, mVpeak, SumSigma, 0, SumSigmaErr);
  ge2->SetTitle("Riemann Sum Width");
  ge2->SetMarkerStyle(20); ge2->Draw("ap");

  canv->cd(3);
  TGraphErrors* ge3 = new TGraphErrors(NoEnt, mVpeak, SumSq, 0, SumSqErr);
  ge3->SetTitle("Sum of Squares");
  ge3->SetMarkerStyle(20); ge3->Draw("ap");

  canv->cd(4);
  TGraphErrors* ge4 = new TGraphErrors(NoEnt, mVpeak, SumSqErr, 0, SumSqSigmaErr);
  ge4->SetTitle("Sum of Squares Width");
  ge4->SetMarkerStyle(20); ge4->Draw("ap");

  canv->Update();
  canv->Print("SummaryPlots.pdf(");
  canv->Clear();
  canv->Divide(2,2);

  canv->cd(1);
  TGraphErrors* ge5 = new TGraphErrors(NoEnt, mVpeak, BoundSum, 0, BoundSumErr);
  ge5->SetTitle("Partial Riemann Sum");  ge5->SetMarkerStyle(20); ge5->Draw("ap");

  canv->cd(2);
  TGraphErrors* ge6 = new TGraphErrors(NoEnt, mVpeak, BoundSumSigma, 0, BoundSumSigmaErr);
  ge6->SetTitle("Partial Riemann Sum Width");  ge6->SetMarkerStyle(20); ge6->Draw("ap");

  canv->cd(3);
  TGraphErrors* ge7 = new TGraphErrors(NoEnt, mVpeak, Peak, 0, PeakErr);
  ge7->SetTitle("Peak Value");  ge7->SetMarkerStyle(20); ge7->Draw("ap");

  canv->cd(4);
  TGraphErrors* ge8 = new TGraphErrors(NoEnt, mVpeak, PeakSigma, 0, PeakSigmaErr);
  ge8->SetTitle("Peak Value Width");  ge8->SetMarkerStyle(20); ge8->Draw("ap");

  canv->Print("SummaryPlots.pdf");
  canv->Clear();
  canv->Divide(2,2);

  canv->cd(1);
  TGraphErrors* ge9 = new TGraphErrors(NoEnt, mVpeak, ToT, 0, ToTErr);
  ge9->SetTitle("Time Over 1/3 Peak");  ge9->SetMarkerStyle(20); ge9->Draw("ap");

  canv->cd(2);
  TGraphErrors* ge10 = new TGraphErrors(NoEnt, mVpeak, ToTSigma, 0, ToTSigmaErr);
  ge10->SetTitle("Time Over 1/3 Peak Width");  ge10->SetMarkerStyle(20); ge10->Draw("ap");

  canv->Print("SummaryPlots.pdf)");
  infile->Close();
}
