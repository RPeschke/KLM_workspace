#include "Riostream.h"
//==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|==|
//  THIS IS A WORK IN PROGRESS. IT IS/HAS BEING/BEEN ADAPTED FROM THE SINGLE
//  CHANNEL VERSION OF THE SAME SCRIPT. I'M THINKING OF COMBINING THIS SCRIPT
//  WITH THE FORMER SUCH THAT THE MODE OF DATA STORAGE CAN BE SELECTED WITH
//  ONE OF THE ARGUMENTS.
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
                            const char* root_file,
                            const char* root_dir,
                            const char* root_tree,
                            const bool  spcSvr=0,
                            Option_t*   tree_desc="TargetX Data"){
  gROOT->Reset();

  Int_t EvtNum, AddNum, TAddNum, WrAddNum, Wctime, ASIC, TASIC;
  Int_t garf1, garf2, pedWin;
  Int_t PeakTime[16], t1_1PE[16], t2_1PE[16];

  Short_t RawSample[16];
  Short_t PedSample[16][512][32];

  Float_t peakAvg, maxSa, s, sl, sr, PeakVal[16];
  Float_t Sample[16][128], FIRsample[16][128], partialQ[16];
  Float_t onePE = 10;

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
  if (!gFile->Get(root_dir)){
    writeFlag = kTRUE;
    printf("Making root directory %s\n", root_dir);
    gFile->mkdir(root_dir,"Preamp Saturation Data");
  }
  if (writeFlag) gFile->Write();

  // Check if TTree exists and make it if not
  gFile->cd(root_dir);
  TTree* tree;
  if (gDirectory->Get(root_tree)){
    tree = (TTree*)gDirectory->Get(root_tree);
    tree->SetBranchAddress("EvtNum",    &EvtNum);
    tree->SetBranchAddress("AddNum",    &TAddNum);
    //tree->SetBranchAddress("WrAddNum",  &WrAddNum);
    //tree->SetBranchAddress("Wctime",    &Wctime);
    tree->SetBranchAddress("ASIC",      &TASIC);
    tree->SetBranchAddress("PeakTime",  PeakTime);
    tree->SetBranchAddress("PeakVal",   PeakVal);
    tree->SetBranchAddress("partialQ",  partialQ);
    tree->SetBranchAddress("TimeStart", t1_1PE);
    tree->SetBranchAddress("TimeStop",  t2_1PE);
    if (!spcSvr) {
      tree->SetBranchAddress("Sample",    Sample);
      tree->SetBranchAddress("FIRsample", FIRsample);
    }
  }
  else {
    tree = new TTree(root_tree, tree_desc);
    tree->Branch("EvtNum",    &EvtNum,   "EvtNum/I");
    tree->Branch("AddNum",    &TAddNum,  "AddNum/I");
    //tree->Branch("WrAddNum",  &WrAddNum, "WrAddNum/I");
    //tree->Branch("Wctime",    &Wctime,   "Wctime/I");
    tree->Branch("ASIC",      &TASIC,    "ASIC/I");
    tree->Branch("PeakTime",  PeakTime,  "PeakTime[16]/I");
    tree->Branch("PeakVal",   PeakVal,   "PeakVal[16]/F");
    tree->Branch("partialQ",  partialQ,  "partialQ[16]/F");
    tree->Branch("TimeStart", t1_1PE,    "TimeStart[16]/I");
    tree->Branch("TimeStop",  t2_1PE,    "TimeStop[16]/I");
    if (!spcSvr) {
      tree->Branch("Sample",    Sample,    "Sample[16][128]/F");
      tree->Branch("FIRsample", FIRsample, "FIRSample[16][128]/F");
    }
  }


//--- ESTABLISH TCANVAS AND THISTOGRAM FOR PLOTTING A FEW WAVEFORMS ---//
//  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
//  canv->Divide(2,2);
//
//  TH1F *hist[4];
//  for (int h=0; h<4; h++) {
//    hist[h] = new TH1F("hist", "Random Waveforms", 128, 0, 128);
//    hist[h]->SetMinimum(-50); hist[h]->SetMaximum(150);
//  }

//--- READ IN DATA FROM ASCII FILE, SUBTRACT PEDS, AND EXTRACT FEATURES ---//
  std::ifstream infile;
  infile.open(ascii_input);

  Int_t nlines = 0;
  if (!infile.good()){
    cout << "Error opening data file\nor file is empty. Exiting . . .\n";
    exit(-1);
  }
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
        TAddNum = AddNum;
        TASIC = ASIC;
        // apply FIR filter
        for(int j=0; j<128; j++) { // from 16 samples before turn-on thresholds
          FIRsample[j] = 0.;
          for (int tap=0; tap<numTaps; tap++) {
            if (j-tap>=0) {
              FIRsample[j] += FIR[tap]*Sample[j-tap];
            }
          }
        }
        // find peak of filtered waveform
        maxSa=-9999.; PeakTime = 0; PeakVal = 0.;
        for(int j=1; j<127; j++) { // sample No.
          sl= FIRsample[j-1];
          s = FIRsample[j]  ;
          sr= FIRsample[j+1];
          if (s>maxSa && (s>=sl) && (s>=sr)){// && (s-sl)<50 && (s-sr)<50){
            maxSa = s;
            PeakTime = j;
            PeakVal = (sl + s +sr)/3.0;
          }
        }
        // calculate partial Riemann sum
        partialQ = 0.0;
        if (PeakTime>31 && PeakTime<96){
          for (int j=(PeakTime-8); j<=(PeakTime+32); j++){
            partialQ += Sample[j];
          }
        }
        else PeakVal = 0.0;

        // measure times rising- and falling- 1PE-crossings using filtered waveform
        t1_1PE = t2_1PE = 0;
        if (PeakVal>onePE && partialQ>0.0) {
          float minDiff1PE = 9999;
          float Diff1PE = 0;
          for (int j=PeakTime; j>25; j--) { // sample No.
            Diff1PE = TMath::Abs(FIRsample[j]-onePE);
            if (Diff1PE < minDiff1PE){
              t1_1PE = j;
              minDiff1PE = Diff1PE;
            }
          }
        }

        // plot some waveforms for sanity check
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
    if (nlines%100==0) printf(".");
    if (nlines!=0 && nlines%8000==0) printf("<--%d\n", nlines);
  }//END READ DATA FROM FILE

  tree->Write();
  infile.close();
  gFile->Close();
  canv->Print("temp/randoWaveforms.pdf");
  delete canv;
  printf("\nRead %d lines.\n",nlines);
  printf("Data written to %s:/%s: %s\n\n",  root_file, root_dir, root_tree);
}
