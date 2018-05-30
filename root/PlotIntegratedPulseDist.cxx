void PlotIntegratedPulseDistAndFit5Peaks(const char* root_file, const char* plotTitle, const float actHV) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);

  Float_t PeakVal[16], RiemannSum[16], TimeOverThresh[16];

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("RiemannSum", RiemannSum);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  TH1F* hist0 = new TH1F("", ";Integrated ADC Counts;", 300,-500,5500);
  hist0->SetTitle(plotTitle);
  hist0->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (RiemannSum[0] != 0) hist0->Fill(RiemannSum[0]);
  }

  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)(hist0->Clone());
  TH1F* clone2 = (TH1F*)(hist0->Clone());
  TH1F* clone3 = (TH1F*)(hist0->Clone());
  TH1F* clone4 = (TH1F*)(hist0->Clone());

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
  hist0->Fit("f0", "RS"); // "R" for fit range
  Mean[0] = f0->GetParameter(1); Error[0] = f0->GetParError(1);
  hist0->Draw();

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
  TPaveStats *stat0 = (TPaveStats*)(hist0->FindObject("stats"));
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


  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf", plotTitle);
  canv->Print(pdfOutfile);
  //delete hist;
  delete canv;
  delete results;
  delete file;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void PlotIntegratedPulseDist(const char* root_file, const char* plotTitle) {

using namespace TMath;
  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat("e");// can use "e" for entries, but adds to every instance of TPaveStats
//  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
//  gStyle->SetStatY(.89);
//  gStyle->SetStatX(1);
//  gStyle->SetStatW(0.15);
//  gStyle->SetStatH(0.1);


  Float_t RiemannSum[16];

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("RiemannSum", RiemannSum);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  TH1I* hist = new TH1I("", "", 300,0,20000);
  hist->SetTitle(plotTitle);
  hist->GetXaxis()->SetTitle("Integrated ADC Counts / Evt");
  hist->GetXaxis()->SetLabelSize(.025);
  hist->GetYaxis()->SetTitle("# of Evts / bin (300 bins)");
  hist->GetYaxis()->SetLabelSize(.025);
  //hist->GetYaxis()->SetTitleOffset(1.1);
  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    //if (PeakVal[0]==*std::max_element(PeakVal,PeakVal+15)){
      hist->Fill(RiemannSum[0]);
    //}
  }

  // Clone oritinal histogram for subsequent fit functions
//  TH1I* clone1 = (TH1I*)(hist->Clone());
//  TH1I* clone2 = (TH1I*)(hist->Clone());

  hist->Draw();


//  TF1 *f2 = new TF1("f2", "gaus", 44, 68);
//  f2->SetLineColor(kBlue);
//  clone1->Fit("f2", "R", "SAMES");
//  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

//  TF1 *f3 = new TF1("f3", "gaus", 77, 103);
//  f3->SetLineColor(kRed);
//  clone2->Fit("f3", "R", "SAMES");
//  clone2->Draw("SAMES");

  // Modify Stat boxes to include ALL fit results
  //canv->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
//  TPaveStats *stat1 = (TPaveStats*)(hist->FindObject("stats"));
//  TPaveStats *stat2 = (TPaveStats*)(clone1->FindObject("stats"));
//  TPaveStats *stat3 = (TPaveStats*)(clone2->FindObject("stats"));
//  if(stat1 && stat2 && stat3) {
//    stat1->SetTextColor(kGreen);
//    stat2->SetTextColor(kBlue);
//    stat3->SetTextColor(kRed);
//    float height = stat1->GetY2NDC() - stat1->GetY1NDC();
//    stat2->SetY1NDC(stat1->GetY1NDC() - height);
//    stat2->SetY2NDC(stat1->GetY1NDC() );
//    stat2->Draw();
//    stat3->SetY1NDC(stat1->GetY1NDC() - 2*height);
//    stat3->SetY2NDC(stat1->GetY1NDC() - height);
//    stat3->Draw();
//  }
  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf", plotTitle);
  canv->Print(pdfOutfile);
  delete canv;

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
