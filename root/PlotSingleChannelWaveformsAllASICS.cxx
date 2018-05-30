void PlotSingleWaveformsAllASICS(const char* root_file, int chNo) {
  gROOT->Reset();
  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];


  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1100, 850);
  canvas1->Divide(4, 3); // make 4 pads per canvas

  char xlabel[18];
  char plotTitle[20];

  // declare memory for 4 histograms & configure
  TH1I* hist[10];
  for (int i=0; i<4; i++){
    hist[i] = new TH1I("", "", 128,0,128);
    hist[i]->GetYaxis()->SetTitleOffset(1.5);
    hist[i]->SetMaximum(700);
    hist[i]->SetMinimum(-300);
    hist[i]->GetYaxis()->SetTitle("ADC Counts");
    sprintf(xlabel, "Ch_%d Sample No.", chNo);
    hist[i]->GetXaxis()->SetTitle(xlabel);
  }



  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("ADC_counts", Sample);

  int numEnt = 1;
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    sprintf(plotTitle, "ASIC: %d, Win: %d-%d", ASIC, AddNum, (AddNum+3)%512);
    int i = e%4;
      hist[i]->SetTitle(plotTitle);
      //hist[i]->Reset();


      for(int j=0; j<128; j++) { // sample No.
         hist[i]->
      }
      canvas1->cd(i+1);
      hist[i]->Draw();

      // Print plots to pdf file
      if (e==3 && numEnt>4) canvas1->Print("waveforms.pdf("); // 1st page

      else if ( (e>3 && e<(numEnt-1) && (e%4)==3) | (numEnt<=4 && e==numEnt-1)) {
        canvas1->Print("waveforms.pdf"); // intermediate pages
      }
      else if (e==(numEnt-1) && numEnt>4) {  // last page
        canvas1->Print("waveforms.pdf)");
      }


  }
}
