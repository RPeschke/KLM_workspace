#include "Riostream.h"
//==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|
//        --- Make Calibration TTrees ---
//    Intended for data taken on one channel of multiple ASICs. This script
//  will save pedestal-subtracted feature-extracted data as TTrees into  the
//  appropriate TDirectory for each channel and ASIC of the common root file.
//  As this script is called on multiple times, it will continue to add new
//  channel directories under each ASIC directory for each channel being tested.
//
//
//  Author: Chris Ketter
//          University of Hawaii at Manoa
//          cketter@hawaii.edu
//
//  Last Modified: 1 Dec. 2017
//
//==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|
void MakeCalibTTrees(const char* ascii_input, const char* root_file, const char* root_dir, const int argvCH, bool spcSvr=0) {
// note, ifstream::open, TFile, and TTree are all
// looking for a pointer, so no dereferencing required.
  gROOT->Reset();

  std::ifstream infile;
  infile.open(ascii_input);



  Int_t EvtNum, AddNum, TAddNum, WrAddNum, Wctime, ASIC, TASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t PeakTime, t1_1PE, t2_1PE;
  Float_t peakAvg, maxSa, s, sl, sr, PeakVal;
  Short_t RawSample;//, ToverTh[16];
  Short_t PedSample[10][16][512][32];


  Float_t Sample[128], FIRsample[128], partialQ;//, totalQ[16], Qsquared[16];
//  Float_t AbsValue[16];
  Float_t mVperADC[10] = {.86094, .75229, .96909, .77299, .77299,
                          .77147, .82140, .75664, .86854, .84978};
  const int numTaps= 15;
//  Float_t FIR[numTaps] = {-0.012832178122348499,
//                          -0.025397888564804155,
//                          -0.005248755460049122,
//                           0.07849981802007348,
//                           0.20685169441641885,
//                           0.3060387201706151,
//                           0.3060387201706151,
//                           0.20685169441641885,
//                           0.07849981802007348,
//                          -0.005248755460049122,
//                          -0.025397888564804155,
//                          -0.012832178122348499};
  Float_t FIR[numTaps] = {-0.010647512477423795,
                          -0.022594720086619575,
                          -0.025150380072849567,
                           0.00027792308284435227,
                           0.062244482688751344,
                           0.14757912292866593,
                           0.22306756204974698,
                           0.25329004505718095,
                           0.22306756204974698,
                           0.14757912292866593,
                           0.062244482688751344,
                           0.00027792308284435227,
                          -0.025150380072849567,
                          -0.022594720086619575,
                          -0.010647512477423795};


//  Float_t mVperADC = 0.814765; // average of 10 ASICs from KLMS_0173
  Float_t fCper_mV = 20.0; //GSa/s / 50 Ohm
  Float_t onePE = 10;

  TFile* file = new TFile(root_file,"UPDATE");      // File order matters:

  char dirName[10][25];
  Bool_t writeFlag = kFALSE;
  for (int A=0; A<5; A++){
    sprintf(dirName[A],"%s/ASIC%d/ch%d", root_dir, A, argvCH);
    if (gFile->Get(dirName[A])) {
      continue;
    }
    else {
      writeFlag = kTRUE;
      printf("Making root directory %s\n", dirName[A]);
      gFile->mkdir(dirName[A],"Preamp Saturation Data");
    }
  }
  if (writeFlag) gFile->Write();

  TTree* pedTree = (TTree*)gFile->Get("pedTree"); // get pedestal file and data first
  pedTree->SetBranchAddress("PedSample", PedSample);// use "gDirectory->pwd();" to see!

  //write data to existing or new TTree
  char calibTree[20];
  sprintf(calibTree, "ch%d_%s", argvCH, root_dir);
  TTree* tree[5];
  for (int A=0; A<5; A++){
    gFile->cd(dirName[A]);
    if (gDirectory->Get(calibTree)){
      tree[A] = (TTree*)gDirectory->Get(calibTree);
//      tree[A]->SetBranchAddress("EvtNum", &EvtNum);
      tree[A]->SetBranchAddress("AddNum", &TAddNum);
//      tree[A]->SetBranchAddress("WrAddNum", &WrAddNum);
//      tree[A]->SetBranchAddress("Wctime", &Wctime);
      tree[A]->SetBranchAddress("ASIC", &TASIC);
      tree[A]->SetBranchAddress("PeakTime_ns", &PeakTime);
      tree[A]->SetBranchAddress("PeakVal_mV", &PeakVal);
      tree[A]->SetBranchAddress("partialQ_fC", &partialQ);
//      tree[A]->SetBranchAddress("totalQ_fC", totalQ);
//      tree[A]->SetBranchAddress("Qsquared_fC2", Qsquared);
//      tree[A]->SetBranchAddress("AbsValue_fC", AbsValue);
//      tree[A]->SetBranchAddress("TimeOverThresh", ToverTh);
      tree[A]->SetBranchAddress("TimeStart_ns", &t1_1PE);
      tree[A]->SetBranchAddress("TimeStop_ns", &t2_1PE);
      if (!spcSvr) {
        tree[A]->SetBranchAddress("Sample_mV", Sample);
        tree[A]->SetBranchAddress("FIRsample_mV", FIRsample);
      }
    }
    else {
      tree[A] = new TTree(calibTree, "Preamplifier saturation data");
//    tree[A]->Branch("EvtNum", &EvtNum, "EvtNum/I");
      tree[A]->Branch("AddNum", &TAddNum, "AddNum/I");
//    tree[A]->Branch("WrAddNum", &WrAddNum, "WrAddNum/I");
//    tree[A]->Branch("Wctime", &Wctime, "Wctime/I");
      tree[A]->Branch("ASIC", &TASIC, "ASIC/I");
      tree[A]->Branch("PeakTime_ns", &PeakTime, "PeakTime_ns/I");
      tree[A]->Branch("PeakVal_mV", &PeakVal, "PeakVal_mV/F");
      tree[A]->Branch("partialQ_fC", &partialQ, "partialQ_fC/F");
//    tree[A]->Branch("totalQ_fC", totalQ, "totalQ_fC[16]/F");
//    tree[A]->Branch("Qsquared_fC2", Qsquared, "Qsquared_fC2[16]/F");
//    tree[A]->Branch("AbsValue_fC", AbsValue, "AbsValue_fC[16]/F");
//    tree[A]->Branch("TimeOverThresh", ToverTh, "TimeOverThresh[16]/F");
      tree[A]->Branch("TimeStart_ns", &t1_1PE, "TimeStart_ns/I");
      tree[A]->Branch("TimeStop_ns", &t2_1PE, "TimeStop_ns/I");
      if (!spcSvr) {
        tree[A]->Branch("Sample_mV", Sample, "Sample_mV[128]/F");
        tree[A]->Branch("FIRsample_mV", FIRsample, "FIRSample_mV[128]/F");
      }
    }
  }

  // plot 1st waveform for sanity
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000); canv->Divide(2,2);

  TH1F *hist[4];
  for (int h=0; h<4; h++) {
    hist[h] = new TH1F("hist", "Random Waveforms", 128, 0, 128);
    hist[h]->SetMinimum(-50); hist[h]->SetMaximum(150);
  }
  Int_t nlines = 0;
  if (!infile.good()){
    cout << "Error opening data file\nor file is empty. Exiting . . .\n";
    exit(-1);
  }
  gFile->cd();
//  gDirectory->pwd();
  pedTree->GetEntry(0);
  while (1) { // loops intil end break is reached
    infile >> EvtNum;
    infile >> AddNum;
    infile >> WrAddNum;
    infile >> Wctime;
    infile >> ASIC;
    gFile->cd(dirName[ASIC]);
    for (int i=0; i<16; i++) {
      infile >> garf1;//firmware peak time
      infile >> garf2;//firmware peak value

      for (int j=0; j<4; j++){
        pedWin = (AddNum+j)%512;
        //if (i==0) cout << int i=0; i<16;pedWin << "\t";
        for (int k=0; k<32; k++){
          infile >> RawSample;
          if (i==argvCH){
            Sample[k+32*j] = mVperADC[ASIC]*(PedSample[ASIC][i][pedWin][k] - RawSample);
//            Sample[k+32*j] = mVperADC*(PedSample[ASIC][i][pedWin][k] - RawSample);
          }
        }
      }
      if (i==argvCH){
        TAddNum = AddNum;
        TASIC = ASIC;

        for(int j=0; j<128; j++) { // from 16 samples before turn-on thresholds
          FIRsample[j] = 0.;
          for (int tap=0; tap<numTaps; tap++) {
            if (j-tap>=0) {
              FIRsample[j] += FIR[tap]*Sample[j-tap];
            }
          }
        }

        maxSa=-9999.; maxSaNo = 0; PeakVal = 0.;
        for(int j=1; j<127; j++) { // sample No.
          sl= FIRsample[j-1];
          s = FIRsample[j]  ;
          sr= FIRsample[j+1];
          if (s>maxSa && (s>=sl) && (s>=sr)){// && (s-sl)<50 && (s-sr)<50){
            maxSa = s;
            maxSaNo = j;
            PeakVal = (sl + s +sr)/3.0;
            //partialQ = Sample[j-4] + Sample[j-3] + Sample[j-2] + PeakVal + Sample[j+2] + Sample[j+3] + Sample[j+4];
          }
        }
        partialQ = 0.0;
        if (maxSaNo>38 && maxSaNo<74){
          for (int j=(maxSaNo-8); j<=(maxSaNo+32); j++){
            partialQ += Sample[j];
          }
        }
        else PeakVal = 0.0;
        //PeakVal  = maxSa;
        PeakTime = maxSaNo;


//        partialQ = 0.0;
        t1_1PE = t2_1PE = 0;
        if (PeakVal>onePE && PeakTime>38 && PeakTime<74) {
          float minDiff1PE = 9999;
          float Diff1PE = 0;
          for (int j=maxSaNo; j>25; j--) { // sample No.
            Diff1PE = TMath::Abs(FIRsample[j]-onePE);
            if (Diff1PE < minDiff1PE){
              t1_1PE = j;
              minDiff1PE = Diff1PE;
            }
            //if (Diff1PE > minDiff1PE && minDiff1PE < 10) break;
          }
        }
//          minDiff1PE = 9999;
//          for (int j=maxSaNo; j<128; j++) { // sample No.
//            Diff1PE = TMath::Abs(Sample[j]-onePE);
//            if (Diff1PE < minDiff1PE) t2_1PE = j;
//            if (Diff1PE > minDiff1PE && minDiff1PE < 10) break;
//          }
//          for (int j=t1_1PE; j<=t2_1PE; j++){
//            partialQ += fCper_mV*Sample[j];
//          }
//        }
//        if (maxSaNo < 96 && maxSaNo >7){
//          for (int j=(maxSaNo-8); j<=(maxSaNo+32); j++){
//            partialQ += fCper_mV*Sample[j];
//          }
//        }
//        totalQ[i] = 0.0;
//        //Qsquared[i] = 0.0;
//        //AbsValue[i] = 0.0;
//        for(int j=0; j<128; j++) { // sample No.
//          totalQ[i] += fCper_mV*Sample[j];
//          //Qsquared[i] += fCper_mV*Sample[j]*Sample[j];
//          //AbsValue[i] += fCper_mV*TMath::Abs(Sample[j]);
//        }
        if (nlines==501 || nlines==503) {
          for (int j=0; j<128; j++) {
            hist[(nlines%500)-1]->SetBinContent(j,Sample[j]);
            hist[(nlines%500)]->SetBinContent(j,FIRsample[j]);
          }
          canv->cd(nlines%500); hist[(nlines%500)-1]->Draw();
          canv->cd(nlines%500+1); hist[(nlines%500)]->Draw();
        }
      }
    }
    tree[ASIC]->Fill();
    //gFile->cd();
    if (!infile.good()) break;
    //if (nlines==0) printf("\n%d %d %d %d %d %d %d %f\n", EvtNum, AddNum, WrAddNum, Wctime, ASIC, garf1, garf2, Sample[0][0]);
    nlines++;
    if (nlines%100==0) printf(".");
    if (nlines!=0 && nlines%8000==0) printf("<--%d\n", nlines);
  }
  canv->Print("data/KLMS_0173a/plots/SomeWaveforms.pdf");

  for (int A=0; A<5; A++){
    gFile->cd(dirName[A]);
    tree[A]->Write();
    gFile->cd();
  }
  printf("\nRead %d lines.\n",nlines);
  infile.close();
  delete canv; gFile->Close();
  //gFile->Write(); gFile->Delete("pedTree;2");
  printf("Channel %d (all ASICs) data written to %s:/%s/ASIC*/ch%d\n\n", argvCH, root_file, root_dir, argvCH);
}
