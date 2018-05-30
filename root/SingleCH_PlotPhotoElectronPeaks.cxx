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

//expected PE peaks, function of nPE and HV
float PEpeak(int nPE, float HV){
  float Y = (11.51*nPE + 8.606)*(HV - 68.9096) - 16.248;
  return Y;
}

void PlotPhotoElectronPeaks(const char* SN,
                            const char* root_dir,
                            const char* root_tree,
                            const int   argASIC,
                            const int   argCH,
                            const float argHV     ){

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);

  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t garf1, garf2, pedWin, maxSaNo;
  Int_t PeakTime, t1_1PE, t2_1PE;
  Int_t maxSa, s, sl, sr, partialQ;

  Short_t Sample[128];

  Float_t PeakVal;
  Float_t FIRsample[128];
  Float_t HV = argHV;

  Char_t root_file[30];
  sprintf(root_file, "data/%s/%s.root", SN, SN);

  TFile* file = new TFile(root_file,"UPDATE");
  if (!gFile->Get(root_dir)) {
    printf("\033[31mCan't find directory %s in %s . . . exiting!\033[0m\n", root_dir, root_file);
    exit(-1);
  }
  file->cd(root_dir);
  if (!gDirectory->Get(root_tree)){
    printf("\033[31mCan't find TTree %s in %s . . . exiting!\033[0m\n", root_tree, root_dir);
    exit(-1);
  }
  TTree* tree = (TTree*)gDirectory->Get(root_tree);
  tree = (TTree*)gDirectory->Get(root_tree);
  tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
  //tree->SetBranchAddress("WrAddNum", &WrAddNum);
  //tree->SetBranchAddress("Wctime", &Wctime);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  tree->SetBranchAddress("partialQ_fC", &partialQ);
  //tree->SetBranchAddress("TimeStart_ns", &t1_1PE);
  //tree->SetBranchAddress("TimeStop_ns", &t2_1PE);
  //tree->SetBranchAddress("Sample_mV", Sample);
  //tree->SetBranchAddress("FIRsample_mV", FIRsample);




  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();
//  canv->Divide(2,2);

  // Declare memory on heap for histogram & configure
  TH1F* hist0 = new TH1F("hist0", "Peak Value Distribution", 300,0,300);
  hist0->GetXaxis()->SetTitle("ADC counts");
  hist0->SetLineColor(13);
//
//  TH1F* hist1 = new TH1F("hist1", "Riemann Sum", 300,-500,7000);
//  hist1->GetXaxis()->SetTitle("Integrated ADC counts");
//  hist1->SetLineColor(13);

//  TH1I* hist2 = new TH1I("hist2", "Partial Riemann Sum", 300,0,30000);
//  hist2->GetXaxis()->SetTitle("Integrated ADC counts");
//  hist2->SetLineColor(13);

//  TH1F* hist3 = new TH1F("hist3", "Sum of Squares", 300,0,3000000);
//  hist3->GetXaxis()->SetTitle("Sum of Squares (ADC counts)^2");
//  hist3->SetLineColor(13);
//
//  TH1F* hist4 = new TH1F("hist4", "Sum of Absolute Values", 300, 0, 8000);
//  hist4->GetXaxis()->SetTitle("Absolute ADC counts");
//  hist4->SetLineColor(13);
//
//  TH1F* hist5 = new TH1F("hist5", "Time Over 1/3 Peak Value", 128,0,128);
//  hist5->GetXaxis()->SetTitle("Time (ns)");
//  hist5->SetLineColor(13);
//
//  TH1F* hist6 = new TH1F("hist6", "Time Over 1/3 Peak, times Peak", 400, 0, 25000);
//  hist6->GetXaxis()->SetTitle("ADC counts x Time (counts x ns)");
//  hist6->SetLineColor(13);

  // Fill Histogram
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (PeakVal != 0.0) hist0->Fill(PeakVal);
//    hist1->Fill(RiemannSum[argCH]);
//    hist2->Fill(partialQ[argCH]);
//    if (partialQ != 0) hist2->Fill(partialQ);
//    hist3->Fill(RiemannSumSq[argCH]);
//    hist4->Fill(AbsValue[argCH]);
//    hist5->Fill(ToverTh[argCH]);
//    hist6->Fill(ToverTh[argCH]*PeakVal[argCH]);
  }

//  canv->Print(plotName);
//  canv->Clear();
//  hist2->Draw();

  hist0->Draw();
  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)hist0->Clone();
  TH1F* clone2 = (TH1F*)hist0->Clone();
  TH1F* clone3 = (TH1F*)hist0->Clone();
  TH1F* clone4 = (TH1F*)hist0->Clone();

  float width = 5;
  Double_t Mean[5], dMean[5], Width[5], dWidth[5];

// 1PE
  float onePE = PEpeak(1, HV); cout << onePE << endl;
  char eqnf1[100];
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*4.612*4.612))");
  TF1 *gaus1PE_i = new TF1("gaus1PE_i", eqnf1, onePE-8, onePE+8);
  gaus1PE_i->SetParameters(3780, onePE);
  gaus1PE_i->SetLineColor(2);
  hist0->Fit("gaus1PE_i", "RS"); // "R" for fit range
  Mean[0] = gaus1PE_i->GetParameter(1);
  hist0->Draw();
  TF1 *gaus1PE = new TF1("gaus1PE", "gaus", (Mean[0]-width), (Mean[0]+width));
  gaus1PE->SetLineColor(2);
  hist0->Fit("gaus1PE", "RS"); // "R" for fit range
  Mean[0]  = gaus1PE->GetParameter(1); dMean[0]  = gaus1PE->GetParError(1);
  Width[0] = gaus1PE->GetParameter(2); dWidth[0] = gaus1PE->GetParError(2);
  hist0->Draw();

//  2PE
  float twoPE = PEpeak(2, HV); cout << twoPE << endl;
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*5.03*5.03))");
  TF1 *gaus2PE_i = new TF1("gaus2PE_i", eqnf1, twoPE-8, twoPE+8);
  gaus2PE_i->SetParameters(760, twoPE);
  gaus2PE_i->SetLineColor(3);
  clone1->Fit("gaus2PE_i", "RS", "SAMES");
  Mean[1] = gaus2PE_i->GetParameter(1);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box
  TF1 *gaus2PE = new TF1("gaus2PE", "gaus", (Mean[1]-width), (Mean[1]+width));
  gaus2PE->SetLineColor(3);
  clone1->Fit("gaus2PE", "RS", "SAMES");
  Mean[1] = gaus2PE->GetParameter(1); dMean[1] = gaus2PE->GetParError(1);
  Width[1] = gaus2PE->GetParameter(2); dWidth[1] = gaus2PE->GetParError(2);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

// 3PE
  float threePE = PEpeak(3, HV); cout << threePE << endl;
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*5.608*5.608))");
  TF1 *gaus3PE_i = new TF1("gaus3PE_i", eqnf1, threePE-8, threePE+8);
  gaus3PE_i->SetParameters(158, threePE);
  gaus3PE_i->SetLineColor(4);
  clone2->Fit("gaus3PE_i", "RS", "SAMES");
  Mean[2] = gaus3PE_i->GetParameter(1);
  clone2->Draw("SAMES");
  TF1 *gaus3PE = new TF1("gaus3PE", "gaus", (Mean[2]-width), (Mean[2]+width));
  gaus3PE->SetLineColor(4);
  clone2->Fit("gaus3PE", "RS", "SAMES");
  Mean[2] = gaus3PE->GetParameter(1); dMean[2] = gaus3PE->GetParError(1);
  Width[2] = gaus3PE->GetParameter(2); dWidth[2] = gaus3PE->GetParError(2);
  clone2->Draw("SAMES");

width = 7;

// 4PE
  float fourPE = PEpeak(4, HV); cout << fourPE << endl;
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*5.977*5.977))");
  TF1 *gaus4PE_i = new TF1("gaus4PE_i", eqnf1, fourPE-8, fourPE+8);
  gaus4PE_i->SetParameters(50, fourPE);
  gaus4PE_i->SetLineColor(6);
  clone3->Fit("gaus4PE_i", "R", "SAMES");
  Mean[3] = gaus4PE_i->GetParameter(1);
  clone3->Draw("SAMES");
  TF1 *gaus4PE = new TF1("gaus4PE", "gaus", (Mean[3]-width), (Mean[3]+width));
  gaus4PE->SetLineColor(6);
  clone3->Fit("gaus4PE", "RS", "SAMES");
  Mean[3] = gaus4PE->GetParameter(1); dMean[3] = gaus4PE->GetParError(1);
  Width[3] = gaus4PE->GetParameter(2); dWidth[3] = gaus4PE->GetParError(2);
  clone3->Draw("SAMES");

// 5PE
  float fivePE = PEpeak(5, HV); cout << fivePE << endl;
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*6.5*6.5))");
  TF1 *gaus5PE_i = new TF1("gaus5PE_i", eqnf1, fivePE-8, fivePE+8);
  gaus5PE_i->SetParameters(8, fivePE);
  gaus5PE_i->SetLineColor(38);
  clone4->Fit("gaus5PE_i", "RS", "SAMES");
  Mean[4] = gaus5PE_i->GetParameter(1); dMean[4] = gaus5PE_i->GetParError(1);
  clone4->Draw("SAMES");
//  TF1 *gaus5PE = new TF1("gaus5PE", "gaus", (Mean[4]-width), (Mean[4]+width));
//  gaus5PE->SetParameters(gaus5PE_i->GetParameter(0), Mean[4],7);
//  gaus5PE->SetLineColor(38);
//  clone4->Fit("gaus5PE", "RS", "SAMES");
//  Mean[4] = gaus5PE->GetParameter(1); dMean[4] = gaus5PE->GetParError(1);
//  Width[4] = gaus5PE->GetParameter(2); dWidth[4] = gaus5PE->GetParError(2);
//  clone4->Draw("SAMES");

  // Modify Stat boxes to include ALL fit results
  gPad->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
  TPaveStats *stat0 = (TPaveStats*)(hist0->FindObject("stats"));
  TPaveStats *stat1 = (TPaveStats*)(clone1->FindObject("stats"));
  TPaveStats *stat2 = (TPaveStats*)(clone2->FindObject("stats"));
  TPaveStats *stat3 = (TPaveStats*)(clone3->FindObject("stats"));
  TPaveStats *stat4 = (TPaveStats*)(clone4->FindObject("stats"));
  if(stat0 && stat1 && stat2 && stat3 && stat4) {
    stat0->SetTextColor(2);
    stat0->Draw();

    stat1->SetTextColor(3);
    stat1->SetY1NDC(stat0->GetY1NDC() - height);
    stat1->SetY2NDC(stat0->GetY1NDC() );
    stat1->Draw();

    stat2->SetTextColor(4);
    stat2->SetY1NDC(stat1->GetY1NDC() - height);
    stat2->SetY2NDC(stat1->GetY1NDC());
    stat2->Draw();

    stat3->SetTextColor(6);
    stat3->SetY1NDC(stat2->GetY1NDC() - height);
    stat3->SetY2NDC(stat2->GetY1NDC());
    stat3->Draw();

    stat4->SetTextColor(38);
    stat4->SetY1NDC(stat3->GetY1NDC() - height);
    stat4->SetY2NDC(stat3->GetY1NDC());
    stat4->Draw();
  }

  //write fit results to new TTree
  TTree* tree1;
  if (gDirectory->Get("PEvsHV")){
    tree1 = (TTree*)gDirectory->Get("PEvsHV");
    tree1->SetBranchAddress("Mean", Mean);
    tree1->SetBranchAddress("dMean", dMean);
    tree1->SetBranchAddress("Width", Width);
    tree1->SetBranchAddress("dWidth", dWidth);
    tree1->SetBranchAddress("HV", &HV);
  }
  else {
    tree1 = new TTree("PEvsHV", "Single PE Gaussian fit results for SiPM pulse distributions");
    tree1->Branch("Mean", Mean, "Mean[5]/D");
    tree1->Branch("dMean", dMean, "dMean[5]/D");
    tree1->Branch("Width", Width, "Width[5]/D");
    tree1->Branch("dWidth", dWidth, "dWidth[5]/D");
    tree1->Branch("HV", &HV, "HV/F");
  }
  tree1->Fill();
  tree1->Write("PEvsHV", TObject::kWriteDelete);

//  canv->Write("PEdist");
//  sprintf(plotName, "data/%s/plots/ASIC%d_ch%d_HV%.2f_PEdist.pdf", SN, argASIC, argCH, argHV);
  char plotName[50];
  sprintf(plotName, "data/%s/plots/ch%d/%s_peakValdist.pdf", SN, argCH, root_tree);
  canv->Print(plotName);

  //file->cd(); file->Write();
//  char namecyc[20];
//  sprintf(namecyc, "%s;*", root_tree);
//  gDirectory->Delete(namecyc);
  delete canv;
  file->Close();
}
