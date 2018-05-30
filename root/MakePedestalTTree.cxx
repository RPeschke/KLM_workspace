#include "Riostream.h"

void MakePedestalTTree(const char* ascii_input, const char* root_file) {
  gROOT->Reset();
  std::ifstream infile;
  infile.open(ascii_input);


  Float_t PedSample[16][512][32];
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, pedWin;
  Int_t PeakTime[16], PeakVal[16];
  Float_t Sample;

  TFile* file = new TFile(root_file,"UPDATE");

  // Get existing tree if it exists
  TTree* tree;
  if (file->Get("pedTemp")){
    cout << "Found existing TTree.\n";
    tree = (TTree*)file->Get("pedTemp");
    tree->SetBranchAddress("PedSample", PedSample);
    tree->SetBranchAddress("ASIC", &ASIC);
  }
  else {
    tree = new TTree("pedTemp", "TargetX Pedestal Data (entry%10=ASIC)");
    tree->Branch("PedSample", PedSample, "PedSample[16][512][32]/F");
    tree->Branch("ASIC", &ASIC, "ASIC/I");
  }
  // Read new samples from file
  Int_t nlines = 0;
  while (1) { // loops intil break is reached
    if(nlines%128==0){for(int i=0;i<16;i++){for(int j=0;j<512;j++){for(int k=0;k<32;k++){PedSample[i][j][k]=0.;}}}}
    infile >>      EvtNum;
    infile >>      AddNum;
    if (!infile.good()) break;
    if (AddNum != (nlines*4)%512){
      printf("\nUnanticipated window number.\nAddNum = %d, expected: %d\nExiting!\n\n",AddNum, (nlines*4)%512);
      exit(-1);
    }
    infile >>      WrAddNum;
    infile >>      Wctime;
    infile >>      ASIC;
    for (int i=0; i<16; i++) {
      infile >>  PeakTime[i];
      infile >>  PeakVal[i];
      if(nlines<10 && i==0) cout << ASIC << " " << i << " " << AddNum << "\t";
      for (int j=0; j<4; j++){
        pedWin = (AddNum+j)%512;
        for (int k=0; k<32; k++){
          infile >> Sample;
          PedSample[i][pedWin][k] += Sample;
          if(nlines<10 && i==0 && j==0 && k<16) cout << PedSample[i][pedWin][k] << " ";
        }
      }
      if(nlines<10 && i==0) cout << "\n";

    }
    nlines++;
    if (nlines%128==0) tree->Fill();
  }

  printf("\n\nsuccessfully read %d lines.\nwriting data to TTree\n",nlines);
  infile.close();

  tree->Write();//"", TObject::kSingleKey);
  file->Close();
}



void AveragePedestalTTree(const char* root_file, const float NoAvgs) {
  gROOT->Reset();

  Float_t PedSample[16][512][32], OutPedSample[16][512][32];
  Int_t ASIC, outASIC;

  TFile* file = new TFile(root_file,"UPDATE");
  TTree* tree = (TTree*)file->Get("pedTemp");
  tree->SetBranchAddress("PedSample", PedSample);
  tree->SetBranchAddress("ASIC", &ASIC);

  TTree* outtree;
  outtree = new TTree("pedTree", "TargetX Pedestal Data (entry=ASIC)");
  outtree->Branch("PedSample", OutPedSample, "PedSample[16][512][32]/F");
  outtree->Branch("ASIC", &outASIC, "ASIC/I");

  for (int n=0; n<10; n++) {
    for(int i=0;i<16;i++){for(int j=0;j<512;j++){for(int k=0;k<32;k++){OutPedSample[i][j][k]=0.;}}}
    for (int e=0; e<tree->GetEntriesFast(); e++) {
      tree->GetEntry(e);
      if (ASIC==n) {
        for (int i=0;  i<16; i++) {
          for (int j=0; j<512; j++) {
            for (int k=0; k<32 ; k++) {
              OutPedSample[i][j][k] += PedSample[i][j][k]/NoAvgs;
            }
          }
        }
        outASIC = ASIC;
      }
    }
    outtree->Fill();
  }
  outtree->Write();
  file->Delete("pedTemp;1");
  file->Close();
}
