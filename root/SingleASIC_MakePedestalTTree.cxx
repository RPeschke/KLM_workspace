#include "Riostream.h"

void MakePedestalTTree(const char *ascii_input) {
  gROOT->Reset();
  std::ifstream infile;
  infile.open(ascii_input);
  //ifstream::open looks for a pointer, so no dereferencing required.
  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC, pedWin;
  Int_t PeakTime[16], PeakVal[16];
  Float_t Samp, PedSample[16][512][32];
  Int_t nl = 0; //number of lines

  TFile *file = new TFile("temp/pedTemp.root","UPDATE");
  TTree *tempTree;

  if (file->Get("pedTemp")) file->Delete("pedTemp;*");

  tempTree = new TTree("pedTemp", "TargetX Pedestal Data");
  tempTree->Branch("RawPedSample", PedSample, "RawPedSample[16][512][32]/F");
  tempTree->Branch("ASIC", &ASIC, "ASIC/I");
  tempTree->Branch("AddNum", &AddNum, "AddNum/I");

  while (1) { // loops intil break is reached
    infile >>      EvtNum;
    infile >>      AddNum;
    if (!infile.good()) break;
    if (AddNum != (nl%128)*4){
      printf("Unanticipated window number.\nAddNum = %d, expected: %d\n", AddNum, nl*4);
      exit(-1);
    }
    infile >>  WrAddNum;
    infile >>  Wctime;
    infile >>  ASIC;
    for (int i=0; i<16; i++) {
      infile >>  PeakTime[i];
      infile >>  PeakVal[i];
      for (int j=0; j<4; j++){
        pedWin = (AddNum+j)%512;
        for (int k=0; k<32; k++){
        infile >> Samp;
        PedSample[i][pedWin][k] = Samp;
        }
      }
    }
    if ((nl%128)==127) tempTree->Fill();
    // exit loop when eof, fail, or bad bit from std::ios is set.
    nl++;
  }
  printf("Successfully read %d lines.\nWriting data to TTree\n\n",nl);
  infile.close();

  tempTree->Write();
  file->Close();
}


void AveragePedestalTTree(const char *root_output) {
  gROOT->Reset();

  // Declare & initialize variables
  Int_t   AddNum, ASIC;
  Float_t RawPedSample[16][512][32], FloatAvgPedSample[16][512][32];
  Short_t AvgPedSample[16][512][32];
  for (int i=0; i<16; i++) {  //initialize arrays
    for (int j=0; j<512; j++){
      for (int k=0; k<32; k++){
        AvgPedSample[i][j][k] = 0;
        FloatAvgPedSample[i][j][k] = 0.0;
      }
    }
  }

  // Get raw pedestal data set
  TFile *oldFile = new TFile("temp/pedTemp.root","READ");
  TTree *pedTemp;
  if (oldFile->Get("pedTemp")) pedTemp = (TTree*)oldFile->Get("pedTemp");
  else {printf("Pedestal data does not exist . . . Exiting!\n");exit(-1);}
  pedTemp->SetBranchAddress("RawPedSample", RawPedSample);
  pedTemp->SetBranchAddress("ASIC", &ASIC);
  pedTemp->SetBranchAddress("AddNum", &AddNum);

  // Build tree/file for averaged pedestal data
  TFile *file = new TFile(root_output,"RECREATE");
  TTree *pedTree;
  if (file->Get("pedTree")) file->Delete("pedTree;*");
  pedTree = new TTree("pedTree", "TargetX Pedestal Data");
  pedTree->Branch("PedSample", AvgPedSample, "PedSample[16][512][32]/S");
  pedTree->Branch("ASIC", &ASIC, "ASIC/I");
  pedTree->Branch("AddNum", &AddNum, "AddNum/I");

  // Read & Average Pedestal Values
  int NoAvg = pedTemp->GetEntriesFast();
  printf("Averaging pedestals %d times\n", NoAvg);
  for (int evt=0; evt<NoAvg; evt++){  //average pedestals
    oldFile->cd();
    pedTemp->GetEntry(evt);
    for (int i=0; i<16; i++) {
      for (int j=0; j<512; j++){
        for (int k=0; k<32; k++){
          FloatAvgPedSample[i][j][k] += RawPedSample[i][j][k]/((float)NoAvg);
        }
      }
    }
  }
  for (int i=0; i<16; i++) {    //round averages to short integers
    for (int j=0; j<512; j++){
      for (int k=0; k<32; k++){
        AvgPedSample[i][j][k] = TMath::Nint(FloatAvgPedSample[i][j][k]);
      }
    }
  }

  printf("Sample (0,0,0): %d\n" , AvgPedSample[0][0][0]);    //sanity check

  file->cd();
  pedTree->Fill();
  pedTree->Write();
  file->Close();
}
