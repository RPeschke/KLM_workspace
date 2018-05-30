#include "Riostream.h"

void MakePedSubtractedTTree(const char* ascii_input, const char* root_file, const char* root_dir, const char* root_tree, Option_t* tree_desc="") {
// note, ifstream::open, TFile, and TTree are all
// looking for a pointer, so no dereferencing required.
  gROOT->Reset();

  std::ifstream infile;
  infile.open(ascii_input);

  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t PeakTime[16], t1[16];
  Float_t maxSa, s, sl, sr;
  Float_t RawSample[32], PeakVal[16], ToverTh[16];
  Float_t Sample[16][128], PedSample[16][512][32];
  Float_t partialQ[16], totalQ[16], Qsquared[16];
  Float_t AbsValue[16];
  Float_t mVperADC[10] = {.86094, .75229, .96909, .77299, .77299,
                          .77147, .82140, .75664, .86854, .84978};
  Float_t fCper_mV = 20.0; //GSa/s / 50 Ohm
  TFile* file = new TFile(root_file,"UPDATE");      // File order matters:
  TTree* pedTree = (TTree*)file->Get("pedTree"); // get pedestal file and data first
  pedTree->SetBranchAddress("PedSample", PedSample);// use "gDirectory->pwd();" to see!
  pedTree->GetEntry(0);

  TDirectory *dir;
  if (file->Get(root_dir)) {
    dir = (TDirectory*)file->Get(root_dir);
  }
  else {
    printf("Making root directory %s\n", root_dir);
    dir = file->mkdir(root_dir,"");
  }

  gDirectory->pwd();
  dir->cd();
  gDirectory->pwd();

  //write data to existing or new TTree
  TTree* tree;
  if (gDirectory->Get(root_tree)){
    tree = (TTree*)gDirectory->Get(root_tree);
    tree->SetBranchAddress("EvtNum", &EvtNum);
    tree->SetBranchAddress("AddNum", &AddNum);
    tree->SetBranchAddress("WrAddNum", &WrAddNum);
    tree->SetBranchAddress("Wctime", &Wctime);
    tree->SetBranchAddress("ASIC", &ASIC);
    tree->SetBranchAddress("PeakTime_ns", PeakTime);
    tree->SetBranchAddress("PeakVal_mV", PeakVal);
    tree->SetBranchAddress("partialQ_fC", partialQ);
    tree->SetBranchAddress("totalQ_fC", totalQ);
    tree->SetBranchAddress("Qsquared_fC2", Qsquared);
    tree->SetBranchAddress("AbsValue_fC", AbsValue);
    tree->SetBranchAddress("TimeOverThresh", ToverTh);
    tree->SetBranchAddress("TimeStart_ns", t1);
    tree->SetBranchAddress("Sample_mV", Sample);
  }
  else {
    tree = new TTree(root_tree, tree_desc);
    tree->Branch("EvtNum", &EvtNum, "EvtNum/I");
    tree->Branch("AddNum", &AddNum, "AddNum/I");
    tree->Branch("WrAddNum", &WrAddNum, "WrAddNum/I");
    tree->Branch("Wctime", &Wctime, "Wctime/I");
    tree->Branch("ASIC", &ASIC, "ASIC/I");
    tree->Branch("PeakTime_ns", PeakTime, "PeakTime_ns[16]/I");
    tree->Branch("PeakVal_mV", PeakVal, "PeakVal_mV[16]/F");
    tree->Branch("partialQ_fC", partialQ, "partialQ_fC[16]/F");
    tree->Branch("totalQ_fC", totalQ, "totalQ_fC[16]/F");
    tree->Branch("Qsquared_fC2", Qsquared, "Qsquared_fC2[16]/F");
    tree->Branch("AbsValue_fC", AbsValue, "AbsValue_fC[16]/F");
    tree->Branch("TimeOverThresh", ToverTh, "TimeOverThresh[16]/F");
    tree->Branch("TimeStart_ns", t1, "TimeStart_ns[16]/I");
    tree->Branch("Sample_mV", Sample, "Sample_mV[16][128]/F");
  }

  Int_t nlines = 0;
  if (!infile.good()){
    cout << "Error opening data file\nor file is empty. Exiting . . .\n";
    exit(-1);
  }
  while (1) { // loops intil end break is reached
    infile >> EvtNum;
    infile >> AddNum;
    infile >> WrAddNum;
    infile >> Wctime;
    infile >> ASIC;
    for (int i=0; i<16; i++) {
      infile >> garf1;//firmware peak time
      infile >> garf2;//firmware peak value

      for (int j=0; j<4; j++){
        pedWin = (AddNum+j)%512;
        //if (i==0) cout << int i=0; i<16;pedWin << "\t";
        for (int k=0; k<32; k++){
          infile >> RawSample[k];
          Sample[i][k+32*j] = mVperADC[ASIC]*(PedSample[i][pedWin][k] - RawSample[k]);
          //totalQ[i] += PedSample[i][pedWin][k] - RawSample[k];
        }
      }

      maxSa=-9999.; maxSaNo = 0;
      for(int j=1; j<127; j++) { // sample No.
        sl= Sample[i][j-1];
        s = Sample[i][j]  ;
        sr= Sample[i][j+1];
        if (s>maxSa && (s>=sl) && (s-sl)<50 && (s>=sr) && (s-sr)<50){
          maxSa = s;
          maxSaNo = j;
        }
      }

      //float t1;
      for (int j=1; j<maxSaNo; j++) { // sample No.
        if (Sample[i][j]>(maxSa/3)) {
          t1[i] = j;
          break;
        }
      }
      for (int j=maxSaNo; j<128; j++) { // sample No.
        if (Sample[i][j]<(maxSa/3)) {
          ToverTh[i] = (float)(j-t1[i]);
          break;
        }
      }

      PeakVal[i]  = maxSa;
      PeakTime[i] = maxSaNo;

      partialQ[i] = 0.0;
      if (maxSaNo < 96 && maxSaNo >7){
        for (int j=(maxSaNo-8); j<=(maxSaNo+32); j++){
          partialQ[i] += fCper_mV*Sample[i][j];
        }
      }
      totalQ[i] = 0.0;
      Qsquared[i] = 0.0;
      AbsValue[i] = 0.0;
      for(int j=0; j<128; j++) { // sample No.
        totalQ[i] += fCper_mV*Sample[i][j];
        Qsquared[i] += fCper_mV*Sample[i][j]*Sample[i][j];
        AbsValue[i] += fCper_mV*TMath::Abs(Sample[i][j]);
      }

    }
    if (!infile.good()) break;
    tree->Fill();
    //if (nlines==0) printf("\n%d %d %d %d %d %d %d %f\n", EvtNum, AddNum, WrAddNum, Wctime, ASIC, garf1, garf2, Sample[0][0]);
    nlines++;
  }
  printf("Read %d lines.\n",nlines);
  infile.close();
  file->Write(root_tree);
  file->Delete("pedTree;2");
  printf("Data written to %s:/%s/%s/n",root_file, root_dir, root_tree);
}
