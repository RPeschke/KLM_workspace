void PlotPulseHeightDist(const char* root_file, const char* plotTitle) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.89);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  gStyle->SetStatH(0.1);

  Int_t PeakVal[16];

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("PeakVal", PeakVal);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();

  // Declare memory on heap for histogram & configure
  TH1I* hist = new TH1I("", "", 300,0,300);
  hist->SetTitle(plotTitle);
  hist->GetXaxis()->SetTitle("ADC Counts");

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    //if (PeakVal[0]==*std::max_element(PeakVal,PeakVal+15)){
      hist->Fill(PeakVal[0]);
    //}
  }

  int binVals[155] = {0};
  for(int b=1; b<150; b++){
    binVals[b] = hist->GetBinContent(b);
    //binVals[b] = (hist->GetBinContent(b-1) + hist->GetBinContent(b) + hist->GetBinContent(b+1)) ;
  }

  int maxL=0, maxR=0, minL=0, minR=0, peakNo=0, valleyNo=0;
  int PeakBin[5] = {0};
  int ValleyBin[5] = {0};
  for (int bin=15; bin<120; bin++){
    maxL = *std::max_element(binVals+bin-4, binVals+1+bin);//note weird syntax: (name + start index, name + 1 + finish index)
    maxR = *std::max_element(binVals+bin, binVals+1+bin+4);
    if (binVals[bin]==maxL && binVals[bin]==maxR) {
      PeakBin[peakNo]=bin;
      peakNo++;
      bin+=4;
    }
/*    minL = *std::min_element(binVals+bin-5, binVals+1+bin);//note weird syntax: (name + start index, name + 1 + finish index)
    minR = *std::min_element(binVals+bin, binVals+1+bin+5);
    if (binVals[bin]==minL && binVals[bin]==minR) {
      ValleyBin[valleyNo]=bin;
      valleyNo++;
      //bin+=4;
    }
*/
  }
//  for (int i=0; i<5; i++){
//    cout << PeakBin[i] << "\t" << ValleyBin[i] << "\n";
//  }

  // Clone oritinal histogram for subsequent fit functions
  TH1I* clone1 = (TH1I*)(hist->Clone());
  TH1I* clone2 = (TH1I*)(hist->Clone());

  TF1 *f1 = new TF1("f1", "gaus", PeakBin[0]-7, PeakBin[0]+7);
  f1->SetLineColor(kBlue);//Green);
  hist->Fit("f1", "R"); // "R" for fit range
  hist->Draw();
/*
  TF1 *f2 = new TF1("f2", "gaus", PeakBin[1]-5, PeakBin[1]+7);
  f2->SetLineColor(kRed);//Blue);
  clone1->Fit("f2", "R", "SAMES");
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

  TF1 *f3 = new TF1("f3", "gaus", PeakBin[2]-6, PeakBin[2]+7);
  f3->SetLineColor(kRed);
  clone2->Fit("f3", "R", "SAMES");
  clone2->Draw("SAMES");
*/
  // Modify Stat boxes to include ALL fit results
  canv->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
  TPaveStats *stat1 = (TPaveStats*)(hist->FindObject("stats"));
  TPaveStats *stat2 = (TPaveStats*)(clone1->FindObject("stats"));
//  TPaveStats *stat3 = (TPaveStats*)(clone2->FindObject("stats"));
  if(stat1){//} && stat2){//} && stat3) {
    stat1->SetTextColor(kBlue);//Green);
//    stat2->SetTextColor(kRed);//Blue);
//    stat3->SetTextColor(kRed);
//    float height = stat1->GetY2NDC() - stat1->GetY1NDC();
//    stat2->SetY1NDC(stat1->GetY1NDC() - height);
//    stat2->SetY2NDC(stat1->GetY1NDC() );
//    stat2->Draw();
//    stat3->SetY1NDC(stat1->GetY1NDC() - 2*height);
//    stat3->SetY2NDC(stat1->GetY1NDC() - height);
//    stat3->Draw();
  }
  char pdfOutfile[50];
  sprintf(pdfOutfile, "%s.pdf", plotTitle);
  canv->Print(pdfOutfile);
  sprintf(pdfOutfile, "%s.png", plotTitle);
  canv->Print(pdfOutfile);
  //delete hist;
  delete canv;
  delete file;
}
