void PlotAllWaveforms(const char* root_file) {
  gROOT->Reset();
  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];


  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canvas1->Divide(4, 4); // make 4 pads per canvas

  // declare memory for 4 histograms & configure
  TH2F* h1[16];
  for (int i=0; i<16; i++){
    h1[i] = new TH2F("", "", 128,0,128, 200,-1000,1000);
    h1[i]->GetYaxis()->SetTitleOffset(1.5);
    h1[i]->GetYaxis()->SetTitle("ADC Counts");
  }

  char xlabel[18];
  char m_coord[12];
  char wlabel[20];
  int plotNum = 1;

  tree->SetBranchAddress("ADC_counts", Sample);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("PeakTime", PeakTime);
  int numEnt = 10;//tree->GetEntriesFast();
  int numPlots = 15*numEnt;
  for(int e=0; e<numEnt; e+=16) {

    tree->GetEntry(e);

    sprintf(wlabel, "Windows %d-%d", AddNum, (AddNum+3)%512);
    for (int i=0; i<15; i++) h1[i]->SetTitle(wlabel);

    for(int i=0; i<15; i++) { // channel No.
      plotNum += 1; //15*e + i +1;
      h1[i]->Reset();
      sprintf(xlabel, "Ch_%d Sample No.", i);
      h1[i]->GetXaxis()->SetTitle(xlabel);

      // label peak value on plot
      if (PeakVal[i] == *std::min_element(PeakVal,PeakVal+16)){
        Double_t xpeak = (Double_t) PeakTime[i];
        Double_t ypeak = (Double_t) PeakVal[i];
        TMarker *m = new TMarker(xpeak, ypeak, 22);
        m->SetMarkerColorAlpha(kBlue, 0.35);
        m->SetMarkerStyle(kPlus);
        m->SetMarkerSize(5);
        h1[i]->GetListOfFunctions()->Add(m);
        // print coordinates on plot
        sprintf(m_coord, "[%3.0f, %3.0f]", xpeak, ypeak);
        TPaveLabel *l = new TPaveLabel(xpeak+3,ypeak-75,xpeak+23,ypeak-25, m_coord);
        h1[i]->GetListOfFunctions()->Add(l);
      }

      for(int j=0; j<128; j++) { // sample No.
        Double_t x = (Double_t) j; // TH2I wants double var's for some reason
        Double_t y = (Double_t) Sample[i][j];
        h1[i]->Fill(x, y);
      }
      canvas1->cd(i+1);
      h1[i]->Draw();

      // Print plots to pdf file
      if (plotNum==15) canvas1->Print("waveforms.pdf("); // 1st page

      else if (plotNum>15 && plotNum<numPlots && (plotNum%15)==0) {
        canvas1->Print("waveforms.pdf"); // intermediate pages
      }
      else if (plotNum==(numPlots)) {  // last page
        canvas1->Print("waveforms.pdf)");
      }

//      delete m;
//      delete l;
    }
  }
  // free up memory declared as "new"
  delete canvas1;
  for (int i=0; i<4; i++){delete h1[i];}
}// note: file closes upon exiting function!
