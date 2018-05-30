#include "Riostream.h"
//==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|
//
//
//  Author: Chris Ketter
//          University of Hawaii at Manoa
//          cketter@hawaii.edu
//
//  Last Modified: 17 Jan 2017
//
//==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|
void MakePedSubtractedTTree(const char* ascii_input,
                            const char* ped_file,
                            const char* SN,
                            const char* root_dir,
                            const char* root_tree,
                            const int   argvCH,
                            const bool  spcSvr=0,
                            Option_t*   tree_desc="TargetX Data"){
  gROOT->Reset();
  printf("\nRunning ROOT macro SingleCH_MakePedSubtractedTTree.cxx:\n\n");

  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t t1_1PE, t2_1PE;
  Int_t maxSa, s, sl, sr, partialQ;

  Short_t RawSample;
  Short_t Sample[128], PedSample[16][512][32];

  Float_t PeakVal;
  Float_t FIRsample[128], dFIRsample[127];
  Float_t onePE = 20;

  Char_t root_file[30];
  sprintf(root_file, "data/%s/%s.root", SN, SN);

  // Finite Impulse Response filter
  const int numTaps= 15;
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

//--- FETCH PEDESTAL VALUES FROM FILE ---//
  TFile* pedfile = new TFile(ped_file, "READ");
  TTree* pedTree = (TTree*)pedfile->Get("pedTree"); // get pedestal file and data first
  pedTree->SetBranchAddress("PedSample", PedSample);// use "gDirectory->pwd();" to see!
  pedTree->GetEntry(0);


//--- ESTABLISH NEW TFILE, TDIRECTORY, AND TTREE ---//
  TFile* file = new TFile(root_file,"UPDATE");

  // Check if TDirectory exists and make it if not
  Bool_t writeFlag = kFALSE;
  printf("Looking for ROOT directory %s . . .\n", root_dir);
  if (!gFile->Get(root_dir)){
    writeFlag = kTRUE;
    printf("Directory does not exist \033[36m---> Making ROOT directory %s\033[0m\n\n", root_dir);
    gFile->mkdir(root_dir,"Preamp Saturation Data");
  }
  else {printf("Found Directory.\n\n");}
  if (writeFlag) gFile->Write();

  // Check if TTree exists and make it if not
  gFile->cd(root_dir);
  TTree* tree;
  printf("Looking for ROOT TTree %s\n", root_tree);
  if (gDirectory->Get(root_tree)){
    printf("Found ROOT TTree.\n\n");
    tree = (TTree*)gDirectory->Get(root_tree);
    tree->SetBranchAddress("EvtNum", &EvtNum);
    tree->SetBranchAddress("AddNum", &AddNum);
    //tree->SetBranchAddress("WrAddNum", &WrAddNum);
    //tree->SetBranchAddress("Wctime", &Wctime);
    tree->SetBranchAddress("ASIC", &ASIC);
    tree->SetBranchAddress("PeakTime_ns", &maxSaNo);
    tree->SetBranchAddress("PeakVal_mV", &PeakVal);
    tree->SetBranchAddress("partialQ_fC", &partialQ);
    tree->SetBranchAddress("TimeStart_ns", &t1_1PE);
    tree->SetBranchAddress("TimeStop_ns", &t2_1PE);
    if (!spcSvr) {
      tree->SetBranchAddress("Sample_mV", Sample);
      tree->SetBranchAddress("FIRsample_mV", FIRsample);
      tree->SetBranchAddress("dFIRsample_mV", dFIRsample);
    }
  }
  else {
    printf("ROOT TTree does not exist. \033[36m---> Making ROOT TTree %s.\033[0m\n\n", root_tree);
    tree = new TTree(root_tree, tree_desc);
    tree->Branch("EvtNum", &EvtNum, "EvtNum/I");
    tree->Branch("AddNum", &AddNum, "AddNum/I");
    //tree->Branch("WrAddNum", &WrAddNum, "WrAddNum/I");
    //tree->Branch("Wctime", &Wctime, "Wctime/I");
    tree->Branch("ASIC", &ASIC, "ASIC/I");
    tree->Branch("PeakTime_ns", &maxSaNo, "PeakTime_ns/I");
    tree->Branch("PeakVal_mV", &PeakVal, "PeakVal_mV/F");
    tree->Branch("partialQ_fC", &partialQ, "partialQ_fC/I");
    tree->Branch("TimeStart_ns", &t1_1PE, "TimeStart_ns/I");
    tree->Branch("TimeStop_ns", &t2_1PE, "TimeStop_ns/I");
    if (!spcSvr) {
      tree->Branch("Sample_mV", Sample, "Sample_mV[128]/S");
      tree->Branch("FIRsample_mV", FIRsample, "FIRSample_mV[128]/F");
      tree->Branch("dFIRsample_mV", dFIRsample, "dFIRSample_mV[128]/F");
    }
  }


//--- ESTABLISH TCANVAS AND THISTOGRAM FOR PLOTTING A FEW WAVEFORMS ---//
//  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
//  canv->Divide(2,2);
//
//  TH1I *hist[4];
//  for (int h=0; h<4; h++) {
//    hist[h] = new TH1I("hist", "Random Waveforms", 128, 0, 128);
//    hist[h]->SetMinimum(-50); hist[h]->SetMaximum(150);
//  }

//--- READ IN DATA FROM ASCII FILE, SUBTRACT PEDS, AND EXTRACT FEATURES ---//
  std::ifstream infile;
  infile.open(ascii_input);

  Int_t nlines = 0;
  printf("Looking for ASCII data file '%s.'\n", ascii_input);
  if (!infile.good()){
    printf("\033[31mError opening %s or file is empty. Exiting . . .\033[0m\n\n", ascii_input);
    exit(-1);
  }
  printf("Found ASCII data file '%s.'\n\nAnalyzing Data . . . \n", ascii_input);
  //gFile->cd();
  while (1) { // loops until end break is reached
    infile >> EvtNum;
    infile >> AddNum;
    infile >> WrAddNum;
    infile >> Wctime;
    infile >> ASIC;
    gFile->cd(root_dir);
    for (int i=0; i<16; i++) {
      infile >> garf1;//firmware peak time
      infile >> garf2;//firmware peak value
      for (int j=0; j<4; j++){//window loop
        pedWin = (AddNum+j)%512;
        for (int k=0; k<32; k++){//sample loop
          infile >> RawSample;
          if (i==argvCH){
            Sample[k+32*j] = (PedSample[i][pedWin][k] - RawSample);
          }
        }
      }
      //--- BEGIN FEATURE EXTRACTION ---//
      if (i==argvCH){
        for(int j=0; j<128; j++) { // from 16 samples before turn-on thresholds
          FIRsample[j] = 0.;
          for (int tap=0; tap<numTaps; tap++) {
            if (j-tap>=0) {
              FIRsample[j] += FIR[tap]*((float)Sample[j-tap]);
            }
          }
          if (j>0) dFIRsample[j]= FIRsample[j]-FIRsample[j-1];
        }

        maxSa=-9999; maxSaNo = 0; PeakVal = 0.0;
        for(int j=1; j<127; j++) { // sample No.
          if (j<32 && FIRsample[j]>3){
            PeakVal=0.0;
            break;
          }
          sl= Sample[j-1];
          s = Sample[j]  ;
          sr= Sample[j+1];
          if (s>maxSa && (s>=sl) && (s>=sr) && (s-sl)<50 && (s-sr)<50) {
            maxSa = s;
            maxSaNo = j;
            PeakVal = (sl + s +sr)/3.0;
            //partialQ = Sample[j-4] + Sample[j-3] + Sample[j-2] + PeakVal + Sample[j+2] + Sample[j+3] + Sample[j+4];
          }
        }
        if (maxSaNo <32 || maxSaNo >95) PeakVal=0.0;
        partialQ = 0;
        if (PeakVal > 0){
          for (int j=(maxSaNo-10); j<=(maxSaNo+32); j++){
            partialQ += Sample[j];
          }
        }
        //else PeakVal = 0.0;
        //PeakVal  = maxSa;


        t1_1PE = t2_1PE = 0;
        if (PeakVal>onePE && partialQ>0) {
          float minDiff1PE = 9999;
          float Diff1PE = 0;
          for (int j=maxSaNo; j>(maxSaNo-15); j--) { // sample No.
            Diff1PE = TMath::Abs(FIRsample[j]-onePE);
            if (Diff1PE < minDiff1PE){
              t1_1PE = j;
              minDiff1PE = Diff1PE;
            }
          }
          minDiff1PE = 9999;
          for (int j=maxSaNo; j<128; j++) { // sample No.
            Diff1PE = TMath::Abs(FIRsample[j]-onePE);
            if (Diff1PE < minDiff1PE){
              t2_1PE = j;
              minDiff1PE = Diff1PE;
            }
          }
        }
        //if (nlines==501 || nlines==503) {
        //  for (int j=0; j<128; j++) {
        //    hist[(nlines%500)-1]->SetBinContent(j,Sample[j]);
        //    hist[(nlines%500)]->SetBinContent(j,FIRsample[j]);
        //  }
        //  canv->cd(nlines%500); hist[(nlines%500)-1]->Draw();
        //  canv->cd(nlines%500+1); hist[(nlines%500)]->Draw();
        //}
      }// END FEATURE EXTRACTION
    }// END CHANNEL LOOP
    tree->Fill();
    //gFile->cd();
    if (!infile.good()) break;
    nlines++;
    if (nlines%100==0) std::cout << "." << flush;
    if (nlines!=0 && nlines%8000==0) printf("<--%d\n", nlines);
  }//END READ DATA FROM FILE

  tree->Write();
  infile.close();
  gFile->Close();
  //canv->Print("temp/randoWaveforms.pdf");
  //delete canv;
  printf("\nSuccessfully analyzed %d events.\n",nlines);
  printf("Data written to %s:/%s: %s\n\n",  root_file, root_dir, root_tree);
}
