void Plot3differentDistributions(const char* root_file, const char* plotTitle) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat();// can use "e" for entries, but adds to every instance of TPaveStats


  Float_t PeakVal[16], RiemannSum[16], TimeOverThresh[16];

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("RiemannSum", RiemannSum);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("TimeOverThresh", TimeOverThresh);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->Divide(2,2);

  // Declare memory on heap for histogram & configure
  TH1F* hist = new TH1F("", ";Integrated ADC Counts;", 300,-500,5500);
  hist->SetTitle(plotTitle);
  hist->SetLineColor(13);

  TH1F* hist1 = new TH1F("", ";Peak ADC Counts;", 300,0,400);
  hist1->SetTitle(plotTitle);
  hist1->SetLineColor(13);

  TH1F* hist2 = new TH1F("", ";Time Over 1/2 Peak Value;", 300,0,128);
  hist2->SetTitle(plotTitle);
  hist2->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (RiemannSum[0] != 0) hist->Fill(RiemannSum[0]);
    hist1->Fill(PeakVal[0]);
    hist2->Fill(TimeOverThresh[0]);
  }

  canv->cd(1); gPad->SetLogy(); hist->Draw();
  canv->cd(2); gPad->SetLogy(); hist1->Draw();
  canv->cd(3); gPad->SetLogy(); hist2->Draw();




/*  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf", plotTitle);
  canv->Print(pdfOutfile);
  //delete hist;
  delete canv;
  delete file;*/
}
