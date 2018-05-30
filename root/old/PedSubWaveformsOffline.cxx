#include "Riostream.h"

void PedSubWaveformsOffline(const char* root_file) {
  gROOT->Reset();
  // declare tree variables
  Int_t AddNum, pAddNum;
  Int_t Sample[16][128], pSample[16][128];

  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  //TFile* pFile = new TFile("calib/KLMS0173_HV74p43_peds.root", "READ");
  TFile* pFile = new TFile("calib/KLMS0173_HV74p43_ASIC3_peds.root", "READ");
  TTree* pTree = (TTree*)pFile->Get("tree");
  TCanvas* canvas1 = new TCanvas("canvas1", "Test Canvas", 1000, 1000);
  canvas1->Divide(4, 4); // make 4 pads per canvas

  // declare memory for 16 histograms & configure
  TH2I* h1[16];
  for (int i=0; i<16; i++){
    h1[i] = new TH2I("", "", 128,0,128, 600,-200,400);
    h1[i]->GetYaxis()->SetTitleOffset(1.5);
    h1[i]->GetYaxis()->SetTitle("ADC Counts");
  }

  char xlabel[18];
  char wlabel[20];
  int pad, plotNum = 0;

  tree->SetBranchAddress("ADC_counts", Sample);
  tree->SetBranchAddress("AddNum", &AddNum);
  pTree->SetBranchAddress("ADC_counts", pSample);

  int numEnt = tree->GetEntriesFast();
  int numPlots = 15*numEnt;
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    pTree->GetEntry(AddNum/4);
    sprintf(wlabel, "Windows %d-%d", AddNum, AddNum + 3);
    for(int i=0; i<15; i++) h1[i]->SetTitle(wlabel);
    for(int i=0; i<15; i++) { // channel No.
      plotNum += 1;
      h1[i]->Reset();
      sprintf(xlabel, "Ch_%d Sample No.", i);
      h1[i]->GetXaxis()->SetTitle(xlabel);

      for(int j=0; j<128; j++) { // sample No.
        Double_t x = (Double_t) j; // TH2I wants double var's for some reason
        Double_t y = Double_t(Sample[i][j]-pSample[i][j]);
        //cout << "entry: " << e << "\tChannel: " << i << "\titeration: " << j << "\tSample: " <<  Sample[i][j] << "\tpSample: " << pSample[i][j] << "\n";
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
      //delete m;
      //delete l;
    }
  }
  // free up memory declared as "new"
  delete canvas1;
  for (int i=0; i<4; i++){delete h1[i];}
}// note: file closes upon exiting function!
