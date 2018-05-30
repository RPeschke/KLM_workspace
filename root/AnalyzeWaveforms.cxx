void AnalyzeWaveforms(const char* root_file, const char* root_dir, const char* root_tree, const int argASIC, const int ch, const float argHV) {
  Int_t AddNum, ASIC;
  Int_t PeakTime, t1;
  Float_t PeakVal, partialQ, Sample[128], FIRsample[128];
  Float_t mVperADC[10] = {.86094, .75229, .96909, .77299, .77299,
                          .77147, .82140, .75664, .86854, .84978};
  //Float_t mVperADC = 0.814765; // average of 10 ASICs from KLMS_0173
  Float_t fCper_mV = 20.0; //GSa/s / 50 Ohm
  Float_t HV = argHV;
  Int_t asic = argASIC;

  TFile* file = new TFile(root_file,"UPDATE");
  if (!gFile->Get(root_dir)) {
    printf("Can't find directory %s in %s . . . exiting!\n", root_dir, root_file);
    exit(-1);
  }
  file->cd(root_dir);
  if (!gDirectory->Get(root_tree)){
    printf("Can't find TTree %s in %s . . . exiting!\n", root_tree, root_dir);
    exit(-1);
  }
  TTree* tree = (TTree*)gDirectory->Get(root_tree);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  tree->SetBranchAddress("partialQ_fC", &partialQ);
  tree->SetBranchAddress("TimeStart_ns", &t1);
  tree->SetBranchAddress("Sample_mV", Sample);
  tree->SetBranchAddress("FIRsample_mV", FIRsample);

  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  TH1F* histPQD = new TH1F("histPQD", "Partial Riemann Sum", 300,0,6000);
  histPQD->GetXaxis()->SetTitle("Integrated ADC counts");
  histPQD->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (partialQ != 0.0) histPQD->Fill(partialQ);
  }

  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)(histPQD->Clone());
  TH1F* clone2 = (TH1F*)(histPQD->Clone());
  TH1F* clone3 = (TH1F*)(histPQD->Clone());
  TH1F* clone4 = (TH1F*)(histPQD->Clone());

  float width = 200.;
  Double_t Mean[5], dMean[5], Width[5], dWidth[5];
  float onePE = HV*4.81853e+02-3.37660e+04;
  float sigma1sq = 215*215;
  char eqnf1[100];

// 1PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*%f))", sigma1sq);
  TF1 *gaus1PE_i = new TF1("gaus1PE_i", eqnf1, (0.6*onePE), (1.4*onePE));
  gaus1PE_i->SetParameters(1250, onePE);
  gaus1PE_i->SetLineColor(2);
  histPQD->Fit("gaus1PE_i", "RS"); // "R" for fit range
  histPQD->Draw();
  Mean[0]  = gaus1PE_i->GetParameter(1); dMean[0]  = gaus1PE_i->GetParError(1);
  TF1 *gaus1PE = new TF1("gaus1PE", "gaus", (Mean[0]-width), (Mean[0]+width));
  gaus1PE->SetLineColor(2);
  histPQD->Fit("gaus1PE", "RS"); // "R" for fit range
  Mean[0]  = gaus1PE->GetParameter(1); dMean[0]  = gaus1PE->GetParError(1);
  Width[0] = gaus1PE->GetParameter(2); dWidth[0] = gaus1PE->GetParError(2);
  histPQD->Draw();

  onePE = Mean[0];
  sigma1sq = Width[0]*Width[0];

//  2PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (4*%f))", sigma1sq);
  TF1 *gaus2PE_i = new TF1("gaus2PE_i", eqnf1, (1.7*onePE), (2.7*onePE));
  gaus2PE_i->SetParameters(350, (2*onePE));
  gaus2PE_i->SetLineColor(3);
  clone1->Fit("gaus2PE_i", "RS", "SAMES");
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box
  Mean[1]  = gaus2PE_i->GetParameter(1); dMean[1]  = gaus2PE_i->GetParError(1);
  Mean[1] = gaus2PE_i->GetParameter(1);
  TF1 *gaus2PE = new TF1("gaus2PE", "gaus", (Mean[1]-width), (Mean[1]+width));
  gaus2PE->SetLineColor(3);
  clone1->Fit("gaus2PE", "RS", "SAMES");
  Mean[1] = gaus2PE->GetParameter(1); dMean[1] = gaus2PE->GetParError(1);
  Width[1] = gaus2PE->GetParameter(2); dWidth[1] = gaus2PE->GetParError(2);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

  // recalculate onePE as average of 1st two peaks
  onePE = Mean[1] - Mean[0];
  sigma1sq = 0.5*Width[1]*Width[1];

// 3PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (6*%f))", sigma1sq);
//  TF1 *gaus3PE_i = new TF1("gaus3PE_i", eqnf1, 1.1*(2*Mean[1]-Mean[0])-0.4*Mean[0], 1.1*(2*Mean[1]-Mean[0])+0.4*Mean[0]);
  TF1 *gaus3PE_i = new TF1("gaus3PE_i", eqnf1, 2.5*onePE, 3.5*onePE);
  gaus3PE_i->SetParameters(95, 3*onePE);
  gaus3PE_i->SetLineColor(4);
  clone2->Fit("gaus3PE_i", "RS", "SAMES");
  clone2->Draw("SAMES");
  Mean[2]  = gaus3PE_i->GetParameter(1); dMean[2]  = gaus3PE_i->GetParError(1);
  TF1 *gaus3PE = new TF1("gaus3PE", "gaus", (Mean[2]-width), (Mean[2]+width));
  gaus3PE->SetLineColor(4);
  clone2->Fit("gaus3PE", "RS", "SAMES");
  Mean[2] = gaus3PE->GetParameter(1); dMean[2] = gaus3PE->GetParError(1);
  Width[2] = gaus3PE->GetParameter(2); dWidth[2] = gaus3PE->GetParError(2);
  clone2->Draw("SAMES");

  sigma1sq = Width[2]*Width[2]/3.0;

// 4PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (8*%f))", sigma1sq);
//  TF1 *gaus4PE_i = new TF1("gaus4PE_i", eqnf1, 1.2*(3*Mean[1]-2*Mean[0])-0.3*Mean[0], 1.2*(3*Mean[1]-2*Mean[0])+0.6*Mean[0]);
  TF1 *gaus4PE_i = new TF1("gaus4PE_i", eqnf1,  3.6*onePE, 4.4*onePE);
  gaus4PE_i->SetParameters(25, 4*onePE);
  gaus4PE_i->SetLineColor(6);
  clone3->Fit("gaus4PE_i", "R", "SAMES");
  clone3->Draw("SAMES");
  Mean[3]  = gaus4PE_i->GetParameter(1); dMean[3]  = gaus4PE_i->GetParError(1);
  TF1 *gaus4PE = new TF1("gaus4PE", "gaus", (Mean[3]-width), (Mean[3]+width));
  gaus4PE->SetLineColor(6);
  clone3->Fit("gaus4PE", "RS", "SAMES");
  Mean[3] = gaus4PE->GetParameter(1); dMean[3] = gaus4PE->GetParError(1);
  Width[3] = gaus4PE->GetParameter(2); dWidth[3] = gaus4PE->GetParError(2);
  clone3->Draw("SAMES");

// 5PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (10*%f))", sigma1sq);
//  TF1 *gaus5PE_i = new TF1("gaus5PE_i", eqnf1, 1.3*(4*Mean[1]-3*Mean[0])-0.4*Mean[0], 1.3*(4*Mean[1]-3*Mean[0])+0.4*Mean[0]);
  TF1 *gaus5PE_i = new TF1("gaus5PE_i", eqnf1, 4.6*onePE, 5.4*onePE);
  gaus5PE_i->SetParameters(6, 5*onePE);
  gaus5PE_i->SetLineColor(38);
  clone4->Fit("gaus5PE_i", "RS", "SAMES");
  clone4->Draw("SAMES");
  Mean[4]  = gaus5PE_i->GetParameter(1); dMean[4]  = gaus5PE_i->GetParError(1);
//  TF1 *gaus5PE = new TF1("gaus5PE", "gaus", (Mean[4]-width), (Mean[4]+width));
//  gaus5PE->SetParameters(gaus5PE_i->GetParameter(0), Mean[4],400);
//  gaus5PE->SetLineColor(38);
//  clone4->Fit("gaus5PE", "RS", "SAMES");
////  Mean[4] = gaus5PE->GetParameter(1); dMean[4] = gaus5PE->GetParError(1);
//  Width[4] = gaus5PE->GetParameter(2); dWidth[4] = gaus5PE->GetParError(2);
//  clone4->Draw("SAMES");
  canv->Print("data/KLMS_0173a/plots/PhotonDistribution.pdf");
  delete canv;
// *** DONE FINDING pe PEAKS

// *** NOW SELECT AND ANALYZE WAVEFORMS
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1100, 850);
//  canvas1->Divide(2, 2); // make 4 pads per canvas

	TPad* TL = new TPad("TL", "", 0.005, 0.505, 0.495, 0.995); TL->Draw();
	TPad* TR = new TPad("TR", "", 0.505, 0.505, 0.995, 0.995); TR->Draw();
	TPad* BL = new TPad("BL", "", 0.005, 0.005, 0.495, 0.495); BL->Draw();
	TPad* BR = new TPad("BR", "", 0.505, 0.005, 0.995, 0.495); BR->Draw();

  char xlabel[18];
  char m_coord[12];
  char wlabel[20];
  int pad, plotNo = 0;


// PAGE ONE *****************************************************************************
// Waveforms before and after FFT

  // PLOTS 1 & 2 ****************
  //find single 3 PE waveform and plot before & after filtering
  TH1F* wave3PE = new TH1F("", "", 128,0,128);
  wave3PE->GetYaxis()->SetTitleOffset(1.5);
  wave3PE->GetYaxis()->SetTitle("Voltage (mV)");
  wave3PE->SetMinimum(-25), wave3PE->SetMaximum(200);
  TH1F* FIRw3PE = new TH1F("", "", 128,0,128);
  FIRw3PE->GetYaxis()->SetTitleOffset(1.5);
  FIRw3PE->GetYaxis()->SetTitle("Voltage (mV)");
  FIRw3PE->SetMinimum(-25), FIRw3PE->SetMaximum(200);
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (TMath::Abs(partialQ-Mean[2])<10){
      sprintf(xlabel,"Ch %d Sample No. (ns)", ch);
      wave3PE->GetXaxis()->SetTitle(xlabel);
      FIRw3PE->GetXaxis()->SetTitle(xlabel);
      wave3PE->SetTitle("Raw 3PE Event");
      FIRw3PE->SetTitle("Filtered");
      for(int j=0; j<128; j++) { // sample No.
        wave3PE->SetBinContent(j,Sample[j]);
        FIRw3PE->SetBinContent(j,FIRsample[j]);
      }
      TL->cd();
      wave3PE->Draw();
      TR->cd();
      FIRw3PE->Draw();
      break;
    }
  }


  // PLOT 3 ****************
  // Fast Fourier Transform of raw 3 PE waveform
	TH1* fftwave3PE = NULL;
	fftwave3PE = wave3PE->FFT(fftwave3PE, "MAG"); // this has units of 1/f_max
  fftwave3PE->SetTitle("FFT of Raw 3PE Waveform");
  fftwave3PE->GetXaxis()->SetTitle("frequency (arbritrary units)");
  BL->cd(); BL->SetLogy();
  fftwave3PE->Draw();

  // PLOT 4 ****************
  // Fast Fourier Transform filtered waveform
	TH1* fftFIRw3PE = NULL;
	fftFIRw3PE = FIRw3PE->FFT(fftFIRw3PE, "MAG"); // this has units of 1/f_max
  fftFIRw3PE->SetTitle("FFT of Filtered Waveform");
  fftFIRw3PE->GetXaxis()->SetTitle("frequency (arbritrary units)");
  BR->cd(); BR->SetLogy();
  fftFIRw3PE->Draw();
  canvas1->Print("data/KLMS_0173a/plots/SingleWaveform.pdf");
  //canvas1->Write("SingleWaveform");
  //canvas1->Clear();

  BL->cd(); BL->SetLogy(0);
  BR->cd(); BR->SetLogy(0);





// PAGE TWO & THREE ************************************************************
// Stacked waveforms with and without filtering for 1, 2, 3, and 4 PE like events

  // Stacked 1 PE waveforms as 2d histogram
  TH2F* hist1PE = new TH2F("", "", 128,0,128, 500,-50,160);
  sprintf(wlabel, "ASIC %d, Ch %d, 1 PE Waveforms", ASIC, ch);
  hist1PE->SetTitle(wlabel);
  hist1PE->GetXaxis()->SetTitle("Time (ns)");
  // Stacked & Filtered 1 PE waveforms as 2d histogram
  TH2F* FIRh1PE = new TH2F("", "", 128,0,128, 500,-50,160);
  FIRh1PE->SetTitle("with FIR filter");
  FIRh1PE->GetXaxis()->SetTitle("Time (ns)");


  // Stacked 2 PE waveforms as 2d histogram
  TH2F* hist2PE = new TH2F("", "", 128,0,128, 500,-50,160);
  sprintf(wlabel, "ASIC %d, Ch %d, 2 PE Waveforms", ASIC, ch);
  hist2PE->SetTitle(wlabel);
  hist2PE->GetXaxis()->SetTitle("Time (ns)");
  // Stacked & Filtered 2 PE waveforms as 2d histogram
  TH2F* FIRh2PE = new TH2F("", "", 128,0,128, 500,-50,150);
  FIRh2PE->SetTitle("with FIR filter");
  FIRh2PE->GetXaxis()->SetTitle("Time (ns)");


  // Stacked 3 PE waveforms as 2d histogram
  TH2F* hist3PE = new TH2F("", "", 128,0,128, 500,-50,160);
  sprintf(wlabel,  "ASIC %d, Ch %d, 3 PE Waveforms", ASIC, ch);
  hist3PE->SetTitle(wlabel);
  hist3PE->GetXaxis()->SetTitle("Time (ns)");
  // Stacked & Filtered 3 PE waveforms as 2d histogram
  TH2F* FIRh3PE = new TH2F("", "", 128,0,128, 500,-50,160);
  FIRh3PE->SetTitle("with FIR filter");
  FIRh3PE->GetXaxis()->SetTitle("Time (ns)");


  // Stacked 4 PE waveforms as 2d histogram
  TH2F* hist4PE = new TH2F("", "", 128,0,128, 500,-50,160);
  sprintf(wlabel, "ASIC %d, Ch %d, 4 PE Waveforms", ASIC, ch);
  hist4PE->SetTitle(wlabel);
  hist4PE->GetXaxis()->SetTitle("Time (ns)");
  // Stacked & Filtered 1 PE waveforms as 2d histogram
  TH2F* FIRh4PE = new TH2F("", "", 128,0,128, 500,-50,160);
  FIRh4PE->SetTitle("with FIR filter");
  FIRh4PE->GetXaxis()->SetTitle("Time (ns)");


  // Align waveforms and fill histograms
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (t1!=0) {
      if (TMath::Abs(partialQ-Mean[0])<100) {
        for(int j=t1-16; j<128; j++) { // from 16 samples before turn-on thresholds
          hist1PE->Fill((float)(j-t1+16), Sample[j]); // fill histogram starting from 0th bin
          FIRh1PE->Fill((float)(j-t1+16), FIRsample[j]); // fill histogram starting from 0th bin
        }
      }
      if (TMath::Abs(partialQ-Mean[1])<100) {
        for(int j=t1-16; j<128; j++) { // from 16 samples before turn-on thresholds
          hist2PE->Fill((float)(j-t1+16), Sample[j]); // fill histogram starting from 0th bin
          FIRh2PE->Fill((float)(j-t1+16), FIRsample[j]); // fill histogram starting from 0th bin
        }
      }
      if (TMath::Abs(partialQ-Mean[2])<100){
        for(int j=t1-16; j<128; j++) {
          hist3PE->Fill((float)(j-t1+16), Sample[j]);
          FIRh3PE->Fill((float)(j-t1+16), FIRsample[j]); // fill histogram starting from 0th bin
        }
      }
      if (TMath::Abs(partialQ-4*Mean[0])<100) {
        for(int j=t1-16; j<128; j++) { // from 16 samples before turn-on thresholds
          hist4PE->Fill((float)(j-t1+16), Sample[j]); // fill histogram starting from 0th bin
          FIRh4PE->Fill((float)(j-t1+16), FIRsample[j]); // fill histogram starting from 0th bin
        }
      }
    }
  }

  TL->cd(); hist1PE->Draw("contz");
  TR->cd(); hist2PE->Draw("contz");
  BL->cd(); FIRh1PE->Draw("contz");
  BR->cd(); FIRh2PE->Draw("contz");
  canvas1->Print("data/KLMS_0173a/plots/1and2peStackedWaveforms.pdf");
  //canvas1->Write("WaveformDistributions");

  TL->cd(); hist3PE->Draw("contz");
  TR->cd(); hist4PE->Draw("contz");
  BL->cd(); FIRh3PE->Draw("contz");
  BR->cd(); FIRh4PE->Draw("contz");
  canvas1->Print("data/KLMS_0173a/plots/3and4peStackedWaveforms.pdf");
  //canvas1->Write("WaveformDistributions");

}
















void AnalyzeBigWaveforms(const char* root_file, const char* root_dir, const char* root_tree, const int argASIC, const int ch, const float argHV) {
  Int_t AddNum, ASIC;
  Int_t PeakTime, t1;
  Float_t PeakVal, partialQ, Sample[128], FIRsample[128];
  Float_t mVperADC[10] = {.86094, .75229, .96909, .77299, .77299,
                          .77147, .82140, .75664, .86854, .84978};
  //Float_t mVperADC = 0.814765; // average of 10 ASICs from KLMS_0173
  Float_t fCper_mV = 20.0; //GSa/s / 50 Ohm
  Float_t HV = argHV;
  Int_t asic = argASIC;

  TFile* file = new TFile(root_file,"UPDATE");
  if (!gFile->Get(root_dir)) {
    printf("Can't find directory %s in %s . . . exiting!\n", root_dir, root_file);
    exit(-1);
  }
  file->cd(root_dir);
  if (!gDirectory->Get(root_tree)){
    printf("Can't find TTree %s in %s . . . exiting!\n", root_tree, root_dir);
    exit(-1);
  }
  TTree* tree = (TTree*)gDirectory->Get(root_tree);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  tree->SetBranchAddress("partialQ_fC", &partialQ);
  tree->SetBranchAddress("TimeStart_ns", &t1);
  tree->SetBranchAddress("Sample_mV", Sample);
  tree->SetBranchAddress("FIRsample_mV", FIRsample);

  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  TH1F* histPVD = new TH1F("histPVD", "Peak Value Distribution", 600,0,600);
  histPVD->GetXaxis()->SetTitle("Peak Value (mV)");
  histPVD->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    //if (partialQ != 0.0) histPVD->Fill(partialQ);
    histPVD->Fill(PeakVal);
  }
  histPVD->Draw();
  canv->Print("data/KLMS_0173a/plots/bigThreshPVD.pdf");
}
