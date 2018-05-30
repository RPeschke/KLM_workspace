#include "Riostream.h"

void FindSatPoint(const char* root_file, const char* root_dir, const char* root_tree, const int argASIC, const int argCH, const int argThresh) { // example: FindSatPoint("KLMS_0173_calib.root", "ASIC4/ch9", "satData")
  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat("e");// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);

  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t PeakTime, t1;
  Float_t PeakVal, ToverTh;
  Float_t Sample[128];
  Float_t partialQ;//, totalQ[16], Qsquared[16];
//  Float_t AbsValue[16];
  Double_t SatPoint;

  TFile* file = new TFile(root_file,"UPDATE");
  TDirectory *dir;
  gDirectory->pwd();
  if (gFile->cd(root_dir)) {
    dir = (TDirectory*)gFile->Get(root_dir);
  }
  else {
    printf("Can't find directory %s in %s . . . exiting!\n", root_dir, root_file);
    exit(-1);
  }

  gFile->cd(root_dir);//dir->cd();
  gDirectory->pwd();

  TTree* tree = (TTree*)gDirectory->Get(root_tree);
//  tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
//  tree->SetBranchAddress("WrAddNum", &WrAddNum);
//  tree->SetBranchAddress("Wctime", &Wctime);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  tree->SetBranchAddress("partialQ_fC", &partialQ);
//  tree->SetBranchAddress("totalQ_fC", totalQ);
//  tree->SetBranchAddress("Qsquared_fC2", Qsquared);
//  tree->SetBranchAddress("AbsValue_fC", AbsValue);
//  tree->SetBranchAddress("TimeOverThresh", ToverTh);
  tree->SetBranchAddress("TimeStart_ns", &t1);
  tree->SetBranchAddress("Sample_mV", Sample);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 850, 1000);
//           canv->SetLogy();
//           canv->Divide(2,2);
  // Declare memory on heap for histogram & configure
  TH1F* hist0 = new TH1F("hist0", "Peak Value Distribution", 600,0,600);
        hist0->GetXaxis()->SetTitle("Peak Voltage (mV)");
        hist0->GetYaxis()->SetTitle("frequency"); hist0->GetYaxis()->SetTitleOffset(1.5);
        hist0->SetLineColor(13);




// FIRST HISTOGRAM **************************
  // Fill Peak-Values Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    hist0->Fill(PeakVal);
  }

  // Fit leading edge to line and use x-intercept to find ADC / TrigDAC ratio
  // First the peak bin is identified, then bin content is scanned from 250 up
  // to the peak bin so that the leading edge of the distribution can isolated
  // from 20% to 80% of the peak. Finally, this region is fitted to a line and
  // the zero-crossing point of this line is stored as the ADC equivalent for
  // the given trigger DAC setting.
  int peakBin, bin10, minDiff10;
  double mVperTrigDAC, dmVperTrigDAC, peakVal;
//  double diffLow, diffHigh, minDiffLow, minDiffHigh, fitBeg, fitEnd;

  peakVal = 0.0;
//  minDiffLow = minDiffHigh = 20000.;
  for (int bin=250; bin<600; bin++) {
    if (hist0->GetBinContent(bin) > peakVal) {
      peakBin = bin;
      peakVal = hist0->GetBinContent(bin);
    }
  }
  for (int bin=peakBin/2; bin<peakBin; bin++) {
    if (TMath::Abs(hist0->GetBinContent(bin)-10) < minDiff10){
      minDiff10 = TMath::Abs(hist0->GetBinContent(bin)-10);
      bin10 = bin;
    }
  }
  for (int bin=bin10; bin>bin10-10; bin--) {
    mVperTrigDAC = (float)bin/(float)argThresh;
    if (TMath::Abs(hist0->GetBinContent(bin)) < 2) break;
  }


//  for (int bin=250; bin<peakBin; bin++){
//    diffLow = TMath::Abs(hist0->GetBinContent(bin)-0.2*peakVal);
//    diffHigh = TMath::Abs(hist0->GetBinContent(bin)-0.8*peakVal);
//    if (diffLow < minDiffLow){
//      minDiffLow = diffLow;
//      fitBeg = bin*1; //ctsPerBin;
//    }
//    if (diffHigh < minDiffHigh){
//      minDiffHigh = diffHigh;
//      fitEnd = bin*1; //ctsPerBin;
//    }
//  }
//  TF1 *f1 = new TF1("f1", "pol1", fitBeg, fitEnd); f1->SetLineColor(kGreen);
//  hist0->Fit("f1", "R"); // "R" for fit range
//
//  mVperTrigDAC  = (-f1->GetParameter(0)/f1->GetParameter(1))/600.0;
//  dmVperTrigDAC =
//    TMath::Sqrt(  TMath::Sq( f1->GetParError(0)/f1->GetParameter(1) )
//                + TMath::Sq( f1->GetParameter(0)*f1->GetParError(1)/TMath::Sq(f1->GetParameter(1)) )
//                )/600.0;
//  cout << "\nADC counts per Trig DAC count: "
//       << mVperTrigDAC
//       << "\nUncertainty: "
//       << dmVperTrigDAC << "\n";




// SECOND & THIRD HISTOGRAMS **************************
  TH2F* hist1 = new TH2F("hist1", "Saturated Waveforms", 128,0,128, 600,0,600);
        hist1->GetXaxis()->SetTitle("Time (ms)");
        hist1->GetYaxis()->SetTitle("Sample Voltage (mV)"); hist1->GetYaxis()->SetTitleOffset(1.4);
        hist1->SetLineColor(13);
  TH2F* hist2 = new TH2F("hist2", "Cut for Samples > 0.9 Max Samp.", 128,0,128, 600,0,600);
        hist2->GetXaxis()->SetTitle("Time (ms)");
        hist2->GetYaxis()->SetTitle("Sample Voltage (mV)"); hist2->GetYaxis()->SetTitleOffset(1.4);
        hist2->SetLineColor(13);

//  dir->cd();
//    float topEvts = 0.0;
//    float binCutoff = {0.0};
//    int b=500;
//    int CH = 0;
//    LOOP: do {
//      subdir[CH]->cd();
//        topEvts += hist0[CH]->GetBinContent(b);
//        if (topEvts >= 150 || b == 300) {
//          binCutoff[CH] = (float)b;
//          b=500; topEvts = 0.0;
//          CH=CH+1;
//          goto LOOP;
//        }
//        b=b-1;
//        goto LOOP;
//    }
//    while (CH<15);

  // Loop through first histogram and find cutoff for for largest events
  float topEvts = 0.0;
  float binCutoff = 0.0;
  int b=peakBin;
  while (b>peakBin-20){
    topEvts += hist0->GetBinContent(b);
    b=b-1;
  }
  cout << "Found " << topEvts << " between " << b << " and peak bin " << peakBin << "\n";
  binCutoff = (float)b;

  // Plot all of largest waveforms superimposed on one graph
  // and replot using only samples > 90% of the peak value
  cout << "Plotting tops of waveforms with peak ADC value > " << binCutoff << "\n";
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    int k=0;
    if (PeakVal>=binCutoff){
      for(int j=t1-16; j<128; j++){
        hist1->Fill((float)k, Sample[j]);
        if (Sample[j]>0.97*PeakVal){
          hist2->Fill((float)j, Sample[j]);
        }
        k++;
      }
    }
  }





// FOURTH HISTOGRAM ***********************************
  // Plot Y projection of 2nd and 3rd histograms on same plot.
  // Fit data from 3rd histogram to gaussian & use (mean + 3*sigma) as saturation point.
  // Save all plots and canvases to appropriate subdirectory in root file.
  TF1* fit1 = new TF1("fit1", "gaus");
  fit1->SetLineColor(1);

  TH1D* hist1_y = hist1->ProjectionY("hist1_y");
        hist1_y->SetLineColor(kSpring - 3);
        hist1_y->SetFillColor(kSpring - 3);
        hist1_y->GetYaxis()->SetTitle("frequency");
        hist1_y->GetYaxis()->SetTitleOffset(1.4);
        hist1_y->SetTitle("Y Projection with/without 90% cut");
  TH1D* hist2_y = hist2->ProjectionY("hist2_y");
        hist2_y->SetLineColor(kAzure - 5);
        hist2_y->SetFillColor(kAzure - 5);
        hist2_y->Fit("fit1");
        SatPoint = fit1->GetParameter(1) + 3*fit1->GetParameter(2);





// DRAW & SAVE PLOTS ***********************

// first histogram
  gPad->SetLogy();
  hist0->Draw();
//  hist0->Write();
  canv->Write("PeakDist");

// second histogram
  canv->Clear();
  gPad->SetLogy(0);
  hist1->Draw("contz");
//  hist1->Write();
  canv->Write("MultWaveform");

// third histogram
  canv->Clear();
  hist2->Draw("contz");
//  hist2->Write();
  canv->Write("MultiWithCut");

// fourth histogram
  canv->Clear();
  hist1_y->Draw();
  hist2_y->Draw("SAMES");
//  hist1_y->Write();
//  hist2_y->Write();
  canv->Write("ProjYwCut");
  delete canv;





// SAVE MEASUREMENTS ***********************
//  gFile->cd(root_dir);

  Int_t asic = argASIC;
  Int_t channel = argCH;
  //char namecycle[30];
  //sprintf(namecycle, "%s/Ch%d_satData;*", root_dir, argCH);
  //gFile->Delete(namecycle);
  // Save measurements to top level in tree
  gFile->cd();
  TTree* measurements;
  if (gFile->Get("measurements")){
    measurements = (TTree*)gFile->Get("measurements");
    measurements->SetBranchAddress("ASIC", &asic);
    measurements->SetBranchAddress("Channel", &channel);
    measurements->SetBranchAddress("SatPoint", &SatPoint);
    measurements->SetBranchAddress("mVperTrigDAC", &mVperTrigDAC);
//    measurements->SetBranchAddress("dmVperTrigDAC", &dmVperTrigDAC);
  }
  else {
    measurements = new TTree("measurements", "SatPoints, ADCperTrigDac, ...");
    measurements->Branch("ASIC", &asic, "ASIC/I");
    measurements->Branch("Channel", &channel, "Channel/I");
    measurements->Branch("SatPoint", &SatPoint, "SatPoint/D");
    measurements->Branch("mVperTrigDAC", &mVperTrigDAC, "mVperTrigDAC/D");
//    measurements->Branch("dmVperTrigDAC", &dmVperTrigDAC, "dmVperTrigDAC/D");
  }
  measurements->Fill();
  measurements->Write("",TObject::kWriteDelete);
//  gFile->Write("",TObject::kWriteDelete);//{ kSingleKey = BIT(0), kOverwrite = BIT(1), kWriteDelete = BIT(2) }
  gFile->Close();
}
