void PlotIntegratedPulseDist(const char* root_file, const char* plotTitle) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat();// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.89);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  gStyle->SetStatH(0.1);
  gStyle->SetPaperSize(20,26);

  Float_t Sample[16][128], PeakVal[16], RiemannSum[16];
  Int_t AddNum, samp;

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("RiemannSum", RiemannSum);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("ADC_counts", Sample);
  tree->SetBranchAddress("AddNum", &AddNum);

  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 850, 1100);
  canv->Divide(1,2);

  // Declare memory on heap for histogram & configure
  TH1F* hist1 = new TH1F("", "", 512,0,511);
  hist1->SetTitle("Storage Window Distribution");
  hist1->GetXaxis()->SetTitle("Window Number");

  TH1F* hist2 = new TH1F("", "", 512,0,511);
  hist2->SetTitle("Storage Windows containing samples with |counts| > 1000");
  hist2->GetXaxis()->SetTitle("Window Number");

  TH1F* hist3 = new TH1F("", "", 128,0,127);
  hist3->SetTitle("Sample numbers with |counts| > 1000");
  hist3->GetXaxis()->SetTitle("Sample Number");

  TH1F* hist4 = new TH1F("", "", 300,-50,200);
  hist4->SetTitle("Sample 15, Win 135 Distribution");
  hist4->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist5 = new TH1F("", "", 300,-50,200);
  hist5->SetTitle("Sample 31, Win 417 Distribution");
  hist5->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist6 = new TH1F("", "", 300,-50,200);
  hist6->SetTitle("Sample 48, Win 280 Distribution");
  hist6->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist7 = new TH1F("", "", 300,-50,200);
  hist7->SetTitle("Sample 61, Win 102 Distribution");
  hist7->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist8 = new TH1F("", "", 300,-50,200);
  hist8->SetTitle("Sample 62, Win 507 Distribution");
  hist8->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist9 = new TH1F("", "", 300,-50,200);
  hist9->SetTitle("Sample 88, Win 399 Distribution");
  hist9->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  TH1F* hist0 = new TH1F("", "", 300,-50,200);
  hist0->SetTitle("Sample 113, Win 14 Distribution");
  hist0->GetXaxis()->SetTitle("Wilkinson ADC Counts");

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    hist1->Fill(AddNum);
    for (samp=0; samp<128; samp++){
      if (Sample[0][samp] < -1000 || Sample[0][samp] > 1000){
        hist2->Fill(AddNum);
        hist3->Fill(samp);
      }
    }
    //if (AddNum==128) hist3->Fill(Sample[0][0]);
    if (AddNum==135) hist4->Fill(Sample[0][15]);
    if (AddNum==417) hist5->Fill(Sample[0][31]);
    if (AddNum==280) hist6->Fill(Sample[0][48]);
    if (AddNum==102) hist7->Fill(Sample[0][61]);
    if (AddNum==507) hist8->Fill(Sample[0][62]);
    if (AddNum==399) hist9->Fill(Sample[0][88]);
    if (AddNum==14) hist0->Fill(Sample[0][113]);
  }

//  int binVals[155] = {0};
//  for(int b=1; b<150; b++){
//    binVals[b] = hist->GetBinContent(b);
    //binVals[b] = (hist->GetBinContent(b-1) + hist->GetBinContent(b) + hist->GetBinContent(b+1)) ;
//  }
/*
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
*/
/*    minL = *std::min_element(binVals+bin-5, binVals+1+bin);//note weird syntax: (name + start index, name + 1 + finish index)
    minR = *std::min_element(binVals+bin, binVals+1+bin+5);
    if (binVals[bin]==minL && binVals[bin]==minR) {
      ValleyBin[valleyNo]=bin;
      valleyNo++;
      //bin+=4;
    }
  }
*/
//  for (int i=0; i<5; i++){
//    cout << PeakBin[i] << "\t" << ValleyBin[i] << "\n";
//  }

  // Clone oritinal histogram for subsequent fit functions
//  TH1I* clone1 = (TH1I*)(hist->Clone());
//  TH1I* clone2 = (TH1I*)(hist->Clone());
/*
  TF1 *f1 = new TF1("f1", "gaus", PeakBin[0]-7, PeakBin[0]+7);
  f1->SetLineColor(kBlue);//Green);
  hist->Fit("f1", "R"); // "R" for fit range
*/
  canv->cd(1); gPad->SetLogy(); hist1->Draw();
  canv->cd(2); gPad->SetLogy(); hist2->Draw();
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
//  canv->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
//  TPaveStats *stat1 = (TPaveStats*)(hist->FindObject("stats"));
//  TPaveStats *stat2 = (TPaveStats*)(clone1->FindObject("stats"));
//  TPaveStats *stat3 = (TPaveStats*)(clone2->FindObject("stats"));
//  if(stat1){//} && stat2){//} && stat3) {
//    stat1->SetTextColor(kBlue);//Green);
//    stat2->SetTextColor(kRed);//Blue);
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
  canv->Print("test.pdf(");
  canv->Clear();
  canv->cd(1); gPad->SetLogy(); hist3->Draw();
  /*canv->cd(2); gPad->SetLogy(); hist4->Draw();
  canv->Print("test.pdf");
  canv->cd(1); gPad->SetLogy(); hist5->Draw();
  canv->cd(2); gPad->SetLogy(); hist6->Draw();
  canv->Print("test.pdf");
  canv->cd(1); gPad->SetLogy(); hist7->Draw();
  canv->cd(2); gPad->SetLogy(); hist8->Draw();
  canv->Print("test.pdf");
  canv->cd(1); gPad->SetLogy(); hist9->Draw();
  canv->cd(2); gPad->SetLogy(); hist0->Draw();*/
  canv->Print("test.pdf)");


  //delete hist;
  delete canv;
  delete file;
}
