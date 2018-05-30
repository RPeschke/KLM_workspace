void PlotMultiChanPulseHeightDist(const char* root_file) {
  gROOT->Reset();

  // declare tree variables
  Int_t ASIC, PeakVal[16];

  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");

  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->Divide(2, 2); // make 4 pads per canvas

  tree->SetBranchAddress("ASIC", &ASIC);
  tree->GetEntry(0);

// declare memory for 4 histograms & configure
  char histTitle[20];
  TH1I* hist[15];
  for (int i=0; i<15; i++){
    sprintf(histTitle, "ASIC %d, Ch %d", ASIC, i);
    hist[i] = new TH1I(histTitle, "", 300,-300,0);
    hist[i]->GetXaxis()->SetTitle("ADC Counts");
  }

  tree->SetBranchAddress("PeakVal", PeakVal);

  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    for(int i=0; i<15; i++) { // channel No.
      if (PeakVal[i]==*std::min_element(PeakVal,PeakVal+15)){
        hist[i]->Fill(PeakVal[i]);
      }
    }
  }

  for (int i=0; i<15; i++){
    canv->cd(i%4+1);
    gPad->SetLogy();
    gStyle->SetStatX(.35);
    //gStyle->SetOptStat(1);
    canv->Update();
    TF1 *f1 = new TF1("f1", "gaus", -16, -40);
    TF1 *f2 = new TF1("f2", "gaus", -62, -42);
    if (i==0){
      gStyle->SetOptStat(11);
      gStyle->SetOptFit(111111);
      canv->Update();
      hist[i]->Fit("f1", "R");
      hist[i]->Draw("PLC");
      hist[i]->Fit("f2", "R");
      hist[i]->Draw("SAME PLC");
      delete f1;
      delete f2;
    }
    else {
      gStyle->SetOptStat(1);
      canv->Update();
      hist[i]->Draw();
    }

    // Print plots to pdf file
    if      (i==3)                    canv->Print("TriggeredPHD.pdf(");// 1st page
    else if (i>3 && i<14 && (i%4)==3) canv->Print("TriggeredPHD.pdf");
    else if (i==14)                   canv->Print("TriggeredPHD.pdf)");//last page
  }
  return;
}
