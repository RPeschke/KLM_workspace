void PlotTriggeredEvents(const char* root_file) {
  gROOT->Reset();

  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum, Wctime, ASIC;
  Int_t PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];

  TFile* file = new TFile(root_file,"READ");

  TTree* tree = (TTree*)file->Get("tree");
  tree->SetBranchAddress("ADC_counts", Sample);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("PeakTime", PeakTime);
  int numEnt = 48;//tree->GetEntriesFast();

  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canvas1->Divide(4, 4); // make 4 pads per canvas


  char xAxisLabel[18];
  char markerCoord[12];
  char windowLabel[20];

  TH2F* hist[16];
  for (int i=0; i<16; i++){
    hist[i] = new TH2F("", "", 128,0,128, 1000,-200,800);
    //hist[i] = new TH2F("", "", 128,0,128, 200,-100,100);
    hist[i]->GetYaxis()->SetTitleOffset(1.5);
    hist[i]->GetYaxis()->SetTitle("ADC Counts");
  }

  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);


    for(int i=0; i<15; i++) {
      if (PeakVal[i] == *std::max_element(PeakVal,PeakVal+16)){

        // label window mumbers on plots
        sprintf(windowLabel, "Windows %d-%d", AddNum, (AddNum+3)%512);
        hist[e%16]->SetTitle(windowLabel);

        // label channel mumber on plot
        sprintf(xAxisLabel, "Ch_%d Sample No.", i);
        hist[e%16]->GetXaxis()->SetTitle(xAxisLabel);

        // mark peak on plot
        Double_t xpeak = (Double_t) PeakTime[i];
        Double_t ypeak = (Double_t) PeakVal[i];
        TMarker *mark = new TMarker(xpeak, ypeak, 22);
        mark->SetMarkerColorAlpha(kBlue, 0.35);
        mark->SetMarkerStyle(kPlus);
        mark->SetMarkerSize(5);
        hist[e%16]->GetListOfFunctions()->Add(mark);

        // print coordinates of peak on plot
        sprintf(markerCoord, "[%3.0f, %3.0f]", xpeak, ypeak);
        TPaveLabel *label = new TPaveLabel(xpeak+3,ypeak+25,xpeak+23,ypeak+75, markerCoord);
        //TPaveLabel *label = new TPaveLabel(xpeak+3,ypeak+4,xpeak+23,ypeak+14, markerCoord);
        hist[e%16]->GetListOfFunctions()->Add(label);

        canvas1->cd(e%16+1);

        // fill histogram with sample values
        for(int j=0; j<128; j++) { // sample No.
          Double_t x = (Double_t) j; // TH2I wants double var's for some reason
          Double_t y = (Double_t) Sample[i][j];
          hist[e%16]->Fill(x, y);
        }

        hist[e%16]->Draw();

        // Print plots to pdf file
        if (e==15 && e!=numEnt-1) {
          canvas1->Print("waveforms.pdf("); // 1st page
          for (int j=0; j<16; j++) hist[j]->Reset();
        }
        else if (e>15 && e<(numEnt-numEnt%16) && (e%16)==15 && e!=numEnt-1 && numEnt!=32) {
          canvas1->Print("waveforms.pdf"); // intermediate pages
          for (int j=0; j<16; j++) hist[j]->Reset();
        }
        else if (e==(numEnt-1)) {  // last page
          if (numEnt>16) canvas1->Print("waveforms.pdf)");
          else canvas1->Print("waveforms.pdf");
        }
        //delete hist;
        //delete mark;
        //delete label;
      }// endif
    }// end channel loop
  }// end event loop
}
