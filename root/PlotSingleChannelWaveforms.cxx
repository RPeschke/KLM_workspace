void PlotSingleChannelWaveforms(const char* root_file, int chNo) {
  gROOT->Reset();
  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];


  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canvas1->Divide(2, 2); // make 4 pads per canvas

  // declare memory for 4 histograms & configure
  TH2F* h1[4];
  for (int i=0; i<4; i++){
    h1[i] = new TH2F("", "", 128,0,128, 200,-300,700);
    h1[i]->GetYaxis()->SetTitleOffset(1.5);
    h1[i]->GetYaxis()->SetTitle("ADC Counts");
  }

  char xlabel[18];
  char m_coord[12];
  char wlabel[20];


  tree->SetBranchAddress("ADC_counts", Sample);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("PeakTime", PeakTime);

  int numEnt = tree->GetEntriesFast();
  if (numEnt >4) numEnt = 4; // limit to 10 page pdf output
  for(int e=0; e<numEnt; e++) {
    if (e%4==0){for (int i=0; i<4; i++){h1[i]->Reset();h1[i]->SetTitle("empty");}}
    tree->GetEntry(e);

    sprintf(wlabel, "Windows %d-%d", AddNum, (AddNum+3)%512);
    sprintf(xlabel, "Ch_%d Sample No.", chNo);
    int i = e%4;
      h1[i]->SetTitle(wlabel);
      //h1[i]->Reset();     
      h1[i]->GetXaxis()->SetTitle(xlabel);

      // label peak value on plot
      if (PeakVal[chNo] == *std::max_element(PeakVal,PeakVal+16)){
        Double_t xpeak = (Double_t) PeakTime[chNo];
        Double_t ypeak = (Double_t) PeakVal[chNo];
        TMarker *m = new TMarker(xpeak, ypeak, 22);
        m->SetMarkerColorAlpha(kBlue, 0.35);
        m->SetMarkerStyle(kPlus);
        m->SetMarkerSize(5);
        h1[i]->GetListOfFunctions()->Add(m);
        // print coordinates on plot
        sprintf(m_coord, "[%3.0f, %3.0f]", xpeak, ypeak);
        TPaveLabel *l = new TPaveLabel(xpeak+3,ypeak+75,xpeak+23,ypeak+25, m_coord);
        h1[i]->GetListOfFunctions()->Add(l);
      }

      for(int j=0; j<128; j++) { // sample No.
        Double_t x = (Double_t) j; // TH2I wants double var's for some reason
        Double_t y = (Double_t) Sample[chNo][j];
        h1[i]->Fill(x, y);
      }
      canvas1->cd(i+1);
      h1[i]->Draw();

      // Print plots to pdf file
      if (e==3 && numEnt>4) canvas1->Print("waveforms.pdf("); // 1st page

      else if ( (e>3 && e<(numEnt-1) && (e%4)==3) | (numEnt<=4 && e==numEnt-1)) {
        canvas1->Print("waveforms.pdf"); // intermediate pages
      }
      else if (e==(numEnt-1) && numEnt>4) {  // last page
        canvas1->Print("waveforms.pdf)");
      }


  }
  // free up memory declared as "new"
  delete canvas1;
  for (int i=0; i<4; i++){delete h1[i];}
}// note: file closes upon exiting function!
