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
  Int_t n = 0;
  //Int_t nlines = 10 * 128;

  TFile *file = new TFile("temp/pedTemp.root","UPDATE");
  TTree *tempTree;

  if (file->Get("pedTemp")) file->Delete("pedTemp;*");

  tempTree = new TTree("pedTemp", "TargetX Pedestal Data");
  tempTree->Branch("RawPedSample", PedSample, "RawPedSample[16][512][32]/F");
  //for (int n=0; n<nlines; n++){
  while (1) { // loops intil break is reached
    //if ((n%128)==0) tree->GetEntry((n/128)%10);
    infile >>      EvtNum;
    infile >>      AddNum;
    if (!infile.good()) break;
    if (AddNum != (n%128)*4){
      printf("Unanticipated window number.\nAddNum = %d, expected: %d\n", AddNum, n*4);
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
        //if (i==0 && pedWin==0 && k==0) cout<<Samp<<"\n";
        }
      }
    }
    if ((n%128)==127) tempTree->Fill();
    // exit loop when eof, fail, or bad bit from std::ios is set.
    n++;
  }
  printf("Successfully read %d lines.\nWriting data to TTree\n\n",n);
  infile.close();

  tempTree->Write();
  file->Close();
}


void AveragePedestalTTree(const char *root_output) {
  gROOT->Reset();
  Float_t RawPedSample[16][512][32], FloatAvgPedSample[16][512][32];
  Short_t AvgPedSample[10][16][512][32];

  TFile *oldFile = new TFile("temp/pedTemp.root","READ");
  TTree *pedTemp;
  if (oldFile->Get("pedTemp")) pedTemp = (TTree*)oldFile->Get("pedTemp");
  else {printf("Pedestal data does not exist . . . Exiting!\n");exit(-1);}
  pedTemp->SetBranchAddress("RawPedSample", RawPedSample);

  TFile *file = new TFile(root_output,"RECREATE");
  TTree *tree;
  if (file->Get("pedTree")) file->Delete("pedTree;*");
  tree = new TTree("pedTree", "TargetX Pedestal Data");
  tree->Branch("PedSample", AvgPedSample, "PedSample[10][16][512][32]/S");

  float NoAvg = (float)(pedTemp->GetEntriesFast())/10;
  cout << NoAvg << "\n";
  for (int asic=0; asic<10; asic++){
    for (int i=0; i<16; i++) {
      for (int j=0; j<512; j++){
        for (int k=0; k<32; k++){
          AvgPedSample[asic][i][j][k] = 0;
          FloatAvgPedSample[i][j][k] = 0;
        }
      }
    }
    for (int evt=0; evt<((int)NoAvg); evt++){
      oldFile->cd();
      pedTemp->GetEntry(10*evt+asic);
      for (int i=0; i<16; i++) {
        for (int j=0; j<512; j++){
          for (int k=0; k<32; k++){
            FloatAvgPedSample[i][j][k] += RawPedSample[i][j][k]/NoAvg;
          }
        }
      }
    }
    for (int i=0; i<16; i++) {
      for (int j=0; j<512; j++){
        for (int k=0; k<32; k++){
          AvgPedSample[asic][i][j][k] = TMath::Nint(FloatAvgPedSample[i][j][k]);
        }
      }
    }
    cout<<AvgPedSample[asic][0][0][0]<<"\n";
  }
  file->cd();
  tree->Fill();
  tree->Write();
//  file->Delete("pedTemp;*");
  file->Close();
}
