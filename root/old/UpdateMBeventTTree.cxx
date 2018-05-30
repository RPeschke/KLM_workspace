#include "Riostream.h"

void UpdateMBeventTTree(const char* ascii_input, const char* root_output) {
  std::ifstream infile;
  infile.open(ascii_input);
  //ifstream::open looks for a pointer, so no dereferencing required.

  Int_t EvtNum, AddNum,  WrAddNum;
  Int_t Wctime, ASIC,    Channel;
  Int_t PeakTime[16], PeakVal[16], Sample[16][128];


  TFile* file = new TFile(root_output,"UPDATE");
  TTree* tree = (TTree*)file->Get("tree");

  tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
  tree->SetBranchAddress("WrAddNum", &WrAddNum);
  tree->SetBranchAddress("Wctime", &Wctime);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime", PeakTime);
  tree->SetBranchAddress("PeakVal", PeakVal);
  tree->SetBranchAddress("ADC_counts", Sample);


  Int_t nlines = 0;

  while (1) { // loops intil break is reached
    infile >>      EvtNum      ;
    infile >>      AddNum      ;
    infile >>      WrAddNum    ;
    infile >>      Wctime      ;
    infile >>      ASIC        ;
    for (int i=0; i<16; i++) {
      infile >>  PeakTime[i] ;
      infile >>  PeakVal[i]  ;
      for (int j=0; j<128; j++) infile >> Sample[i][j];
    }
    // exit loop when eof, fail, or bad bit from std::ios is set.
    if (!infile.good()) break;
    tree->Fill(); // MIGHT NEED TO MOVE THIS UP ONE LINE, NEED TO INVESTIGATE
    nlines++;     // THERE IS ONE SPACE AFTER LAST SAMPLE
  }
  printf("Read %d lines.\n",nlines);
  infile.close();
  tree->Write("tree", TObject::kWriteDelete); // append to existing data, write as new cycle and delete previous cycle
  file->Close();
  delete file;
}
