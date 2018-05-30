#include "Riostream.h"

void MakeAveragedPedestalTTree(const char* raw_root_input, const char* root_output) {
  gROOT->Reset();
  //ifstream::open looks for a pointer, so no dereferencing required.
  Int_t AddNum, Sample[16][128];
  Float_t AvgPedSample[16][512][32];
  for (int chan=0; chan<16; chan++){
    for (int wndw=0; wndw<512; wndw++){
      for (int samp=0; samp<128; samp++){
        AvgPedSample[chan][wndw][samp%32] = 0;
      }
    }
  }

  TFile* file = new TFile(raw_root_input,"READ");
  TTree* tree = (TTree*)file->Get("tree");

  Int_t numEnt = tree->GetEntriesFast();
  if (numEnt%128 != 0){
    printf("Error: Invalid number of entries detected.\nShould be an integer multiple of 128.\nExiting . . .");
    exit(-1);
  }
  Int_t numAvgs = numEnt/128;

  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("ADC_counts", Sample);

  for (int e=0; e<numEnt; e++){
    tree->GetEntry(e);
    for (int chan=0; chan<16; chan++){
      for (int samp=0; samp<128; samp++){
        AvgPedSample[chan][AddNum][samp%32] +=  (float)Sample[chan][samp]/(float)numAvgs;
      }
    }
  }
  file->Close();


  // Write averaged pedestal data to new root file
  TFile* pedFile = new TFile(root_output,"RECREATE");

  TTree* pedTree = new TTree("pedTree","TargetX Pedestal Data");
  pedTree->Branch("PedSample", AvgPedSample, "PedSample[16][512][32]/F");

  pedTree->Fill();
  pedFile->Write();

  pedFile->Close();
}
