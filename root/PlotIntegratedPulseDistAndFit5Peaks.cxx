void PlotIntegratedPulseDistAndFit5Peaks(const char* root_file, const char* plotTitle, const Float_t actHV) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  Float_t height = 0.1;
  gStyle->SetStatH(height);

  // Declare tree variables & get data from file
  Float_t PeakVal[16], RiemannSum[16], TimeOverThresh[16];
  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("RiemannSum", RiemannSum);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  Int_t nbins = 400;
  Int_t xmax = 6000;
  Double_t scale = (double)xmax/(double)nbins;
  TH1F* hist0 = new TH1F("", ";Integrated ADC Counts;", nbins,0,xmax);
  hist0->SetTitle(plotTitle);
  hist0->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (RiemannSum[0] != 0) hist0->Fill(RiemannSum[0]);
  }

  // Find peaks
  Double_t * source = new Double_t[nbins];
  Double_t * dest = new Double_t[nbins];
  for (i = 0; i < nbins; i++){
    source[i]=hist0->GetBinContent(i + 1);
  }
  Int_t npeaks = 3;
  TSpectrum *s = new TSpectrum(2*npeaks);
  Int_t nfound = s->SearchHighRes(source, dest, nbins, 1.5, 1, kTRUE, 3, kTRUE, 1);
  Double_t exp1PE = scale*s->GetPositionX()[0];
  Double_t exp2PE = scale*s->GetPositionX()[1];
  Double_t exp3PE = scale*s->GetPositionX()[2];
  Double_t exp4PE = (5*exp3PE - 4*exp2PE + exp1PE)/2;
//  Double_t exp4PE = scale*s->GetPositionX()[3];
  Double_t exp5PE = (5*exp3PE - 6*exp2PE + 2*exp1PE);

  delete [] source;
  delete [] dest;
  printf("Found %d candidate peaks to fit\n",nfound);


  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)(hist0->Clone());
  TH1F* clone2 = (TH1F*)(hist0->Clone());
  TH1F* clone3 = (TH1F*)(hist0->Clone());
  TH1F* clone4 = (TH1F*)(hist0->Clone());

  Float_t width = 250.;
  Double_t Mean[5], Error[5];
  Float_t HV = actHV;
//  Float_t exp1PE = HV*4.81853e+02-3.37660e+04;
//  Float_t exp2PE = HV*9.78835e+02-6.84374e+04;
//  Float_t exp3PE = HV*1.54434e+03-1.07921e+05;
//  Float_t exp4PE = HV*2.12429e+03-1.48415e+05;
//  Float_t exp5PE = HV*2.46639e+03-1.71947e+05;

  // Fit peaks to gaussians & add to plot
  TF1 *f0 = new TF1("f0", "gaus", (exp1PE-width), (exp1PE+width));
  f0->SetLineColor(2);
  hist0->Fit("f0", "RS"); // "R" for fit range
  Mean[0] = f0->GetParameter(1); Error[0] = f0->GetParError(1);
  hist0->Draw();

  TF1 *f1 = new TF1("f1", "gaus", (exp2PE-width), (exp2PE+width));
  f1->SetLineColor(3);
  clone1->Fit("f1", "RS", "SAMES");
  Mean[1] = f1->GetParameter(1); Error[1] = f1->GetParError(1);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

  TF1 *f2 = new TF1("f2", "gaus", (exp3PE-width), (exp3PE+width));
  f2->SetLineColor(4);
  clone2->Fit("f2", "RS", "SAMES");
  Mean[2] = f2->GetParameter(1); Error[2] = f2->GetParError(1);
  clone2->Draw("SAMES");

  TF1 *f3 = new TF1("f3", "gaus", (exp4PE-width), (exp4PE+width));
  f3->SetLineColor(6);
  clone3->Fit("f3", "RS", "SAMES");
  Mean[3] = f3->GetParameter(1); Error[3] = f3->GetParError(1);
  clone3->Draw("SAMES");

  TF1 *f4 = new TF1("f4", "gaus", (exp5PE-width), (exp5PE+width));
  f4->SetLineColor(38);
  clone4->Fit("f4", "RS", "SAMES");
  Mean[4] = f4->GetParameter(1); Error[4] = f4->GetParError(1);
  clone4->Draw("SAMES");

  TH1F* clonePoly = (TH1F*)(hist0->Clone());
  TF1* f5poly= new TF1("f5poly", "pol10");
  clonePoly->Fit("f5poly", "S", "SAMES");
  clonePoly->Draw("SAMES");

  // Modify Stat boxes to include ALL fit results
  gPad->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
  TPaveStats *stat0 = (TPaveStats*)(hist0->FindObject("stats"));
  TPaveStats *stat1 = (TPaveStats*)(clone1->FindObject("stats"));
  TPaveStats *stat2 = (TPaveStats*)(clone2->FindObject("stats"));
  TPaveStats *stat3 = (TPaveStats*)(clone3->FindObject("stats"));
  TPaveStats *stat4 = (TPaveStats*)(clone4->FindObject("stats"));
  if(stat0 && stat1 && stat2 && stat3 && stat4) {
//  if(stat0 && stat1 && stat2 && stat3) {
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

  // Write fit results to new TTree
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


  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf", plotTitle);
  canv->Print(pdfOutfile);
  //delete hist;
  delete canv;
  delete results;
  delete file;
}
