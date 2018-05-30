void CreateSingleChanPulseHeightDist(const char* root_file, const char* hist_title, int chan) {

  gROOT->Reset(); // may prove helpful when running multiple macros during one session

  // declare tree variables
  Int_t PeakVal[16];

  // declare program-scope variables
  Int_t numEnt = 0;
  Double_t triggeredPH = 0.;
  Int_t ch = (Int_t)chan;
  Char_t outfile[30];

  TFile* file = new TFile(root_file);
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canvas2 = new TCanvas("canvas2", "Test Canvas", 1000, 1000);
  canvas2->SetLogy();

  // declare memory for histogram & configure
  TH1I* h2 = new TH1I(hist_title, "ASIC 3, Ch 0 Triggered PHD", 650,0,650);
  h2->GetXaxis()->SetTitle("ADC Counts");

  tree->SetBranchAddress("PeakVal", PeakVal);
  numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    triggeredPH = (Double_t)PeakVal[ch];
    h2->Fill(triggeredPH);
  }
  h2->Draw();

  // print plot to pdf file
  sprintf(outfile, "TriggeredPHD_Ch%d.pdf", ch);
  canvas2->Print(outfile);

  // free up memory declared as "new"
  delete canvas2;
  delete h2;
}// note: file closes upon exiting function!
