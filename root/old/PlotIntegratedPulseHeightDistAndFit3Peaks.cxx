void PlotIntegratedPulseDistAndFit3Peaks(const char* root_file, const char* plotTitle) {

using namespace TMath;
  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
//  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
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
