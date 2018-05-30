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

void MultiGaussFit(const char* root_file,
                   const int   argASIC,
                   const int   argCH,
                   const float approxHV){

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);


  Int_t PeakVal[16];

  TFile* file = new TFile(root_file,"UPDATE");
  TTree* tree = (TTree*)gDirectory->Get("tree");
  //tree->SetBranchAddress("EvtNum", &EvtNum);
  //tree->SetBranchAddress("AddNum", &AddNum);
  //tree->SetBranchAddress("WrAddNum", &WrAddNum);
  //tree->SetBranchAddress("Wctime", &Wctime);
  //tree->SetBranchAddress("ASIC", &ASIC);
  //tree->SetBranchAddress("ADC_counts", Sample);
  //tree->SetBranchAddress("FeatExtActivated", FeatureExtractionActivated);
  tree->SetBranchAddress("PeakVal", PeakVal);
  //tree->SetBranchAddress("NumSampsInTopEighth", StrtnCnts);
  //tree->SetBranchAddress("Time", Time);
  //tree->SetBranchAddress("AfterPulseFlag", AfterPulseFlag);
  //tree->SetBranchAddress("AfterPulseRelativeAmplitude", AfterPulseRelativeAmplitude);
  //tree->SetBranchAddress("TimeOverThr", TimeOverThr);




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
    if (PeakVal[argCH] != 0.0) hist0->Fill(PeakVal[argCH]);
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
  float sigma1 = 4.697, sigma2 = 5.068, sigma3 = 5.347, sigma4 = 5.902, sigma5 = 6.2;
  float onePE = PEpeak(1, approxHV);

// Multi-Gauss equation, 4 fitting parameters:
    // par0: MultiGauss amplitude
    // par1: Optical cross talk probability
    // par2: Avg. peak separation in ADC counts (equiv. to 1PE)
    // par3: Pedestal offset (additional ped. offset beyond pedestal subtraction
    //       always present in single photon spectra & not completely understood)
  char eqnMG[500];
  sprintf(eqnMG, "[0]*("
    "TMath::Power([1],1)*TMath::Exp(   -.5*TMath::Power( (x-(1*[2]+[3]))/%f,2 )   ) +"
    "TMath::Power([1],2)*TMath::Exp(   -.5*TMath::Power( (x-(2*[2]+[3]))/%f,2 )   ) +"
    "TMath::Power([1],3)*TMath::Exp(   -.5*TMath::Power( (x-(3*[2]+[3]))/%f,2 )   ) +"
    "TMath::Power([1],4)*TMath::Exp(   -.5*TMath::Power( (x-(4*[2]+[3]))/%f,2 )   ) +"
    "TMath::Power([1],5)*TMath::Exp(   -.5*TMath::Power( (x-(5*[2]+[3]))/%f,2 )   ) )",sigma1,sigma2,sigma3,sigma4,sigma5);
  TF1 *MultiGauss = new TF1("MultiGauss", eqnMG, 35, 300);
  MultiGauss->SetParameters(15000,.15,onePE,16);
  hist0->Fit("MultiGauss", "RS");
  float MG_Ampl = MultiGauss->GetParameter(0), dMG_Ampl = MultiGauss->GetParError(0);
  float OCTprob = MultiGauss->GetParameter(1), dOCTprob = MultiGauss->GetParError(1);
  for(int i=0; i<5; i++) Mean[i]=(i+1)*MultiGauss->GetParameter(2) + MultiGauss->GetParameter(3);
  hist0->Draw();

// 1PE
  TF1 *gaus1PE = new TF1("gaus1PE", "gaus", (Mean[0]-5), (Mean[0]+8));
  gaus1PE->SetLineColor(2);
  hist0->Fit("gaus1PE", "RS"); // "R" for fit range
  Mean[0]  = gaus1PE->GetParameter(1); dMean[0]  = gaus1PE->GetParError(1);
  Width[0] = gaus1PE->GetParameter(2); dWidth[0] = gaus1PE->GetParError(2);
  hist0->Draw();

//  2PE
  TF1 *gaus2PE = new TF1("gaus2PE", "gaus", (Mean[1]-5), (Mean[1]+8));
  gaus2PE->SetLineColor(3);
  clone1->Fit("gaus2PE", "RS", "SAMES");
  Mean[1] = gaus2PE->GetParameter(1); dMean[1] = gaus2PE->GetParError(1);
  Width[1] = gaus2PE->GetParameter(2); dWidth[1] = gaus2PE->GetParError(2);
  clone1->Draw("SAMES"); // "sames" prevents overwriting of stats box

// 3PE
  TF1 *gaus3PE = new TF1("gaus3PE", "gaus", (Mean[2]-5), (Mean[2]+8));
  gaus3PE->SetLineColor(4);
  clone2->Fit("gaus3PE", "RS", "SAMES");
  Mean[2] = gaus3PE->GetParameter(1); dMean[2] = gaus3PE->GetParError(1);
  Width[2] = gaus3PE->GetParameter(2); dWidth[2] = gaus3PE->GetParError(2);
  clone2->Draw("SAMES");

// 4PE
  TF1 *gaus4PE = new TF1("gaus4PE", "gaus", (Mean[3]-5), (Mean[3]+11));
  gaus4PE->SetLineColor(6);
  clone3->Fit("gaus4PE", "RS", "SAMES");
  Mean[3] = gaus4PE->GetParameter(1); dMean[3] = gaus4PE->GetParError(1);
  Width[3] = gaus4PE->GetParameter(2); dWidth[3] = gaus4PE->GetParError(2);
  clone3->Draw("SAMES");

// 5PE
  TF1 *gaus5PE = new TF1("gaus5PE", "gaus", (Mean[4]-3), (Mean[4]+13));
  gaus5PE->SetLineColor(38);
  clone4->Fit("gaus5PE", "RS", "SAMES");
  Mean[4] = gaus5PE->GetParameter(1); dMean[4] = gaus5PE->GetParError(1);
  Width[4] = gaus5PE->GetParameter(2); dWidth[4] = gaus5PE->GetParError(2);
  clone4->Draw("SAMES");


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
  //TTree* tree1;
  //if (gDirectory->Get("PEvsHV")){
  //  tree1 = (TTree*)gDirectory->Get("PEvsHV");
  //  tree1->SetBranchAddress("Mean", Mean);
  //  tree1->SetBranchAddress("dMean", dMean);
  //  tree1->SetBranchAddress("Width", Width);
  //  tree1->SetBranchAddress("dWidth", dWidth);
  //  tree1->SetBranchAddress("HV", &approxHV);
  //  tree1->SetBranchAddress("OCTprob", &OCTprob);
  //  tree1->SetBranchAddress("dOCTprob", &dOCTprob);
  //  tree1->SetBranchAddress("MG_Ampl", &MG_Ampl);
  //  tree1->SetBranchAddress("dMG_Ampl", &dMG_Ampl);
  //}
  //else {
  //  tree1 = new TTree("PEvsHV", "Single PE Gaussian fit results for SiPM pulse distributions");
  //  tree1->Branch("Mean", Mean, "Mean[5]/D");
  //  tree1->Branch("dMean", dMean, "dMean[5]/D");
  //  tree1->Branch("Width", Width, "Width[5]/D");
  //  tree1->Branch("dWidth", dWidth, "dWidth[5]/D");
  //  tree1->Branch("HV", &approxHV, "HV/F");
  //  tree1->Branch("OCTprob", &OCTprob, "OCTprob/F");
  //  tree1->Branch("dOCTprob", &dOCTprob, "dOCTprob/F");
  //  tree1->Branch("MG_Ampl", &MG_Ampl, "MG_Ampl/F");
  //  tree1->Branch("dMG_Ampl", &dMG_Ampl, "dMG_Ampl/F");
  //}
  //tree1->Fill();
  //tree1->Write("PEvsHV", TObject::kWriteDelete);

//  canv->Write("PEdist");
//  sprintf(plotName, "data/%s/plots/ASIC%d_ch%d_HV%.2f_PEdist.pdf", SN, argASIC, argCH, argHV);
//  char plotName[50];
//  sprintf(plotName, "data/%s/plots/ch%d/%s_peakValdist.pdf", SN, argCH, root_tree);
//  canv->Print(plotName);

  //char plotName[50];
  //sprintf(plotName, "data/%s/plots/ch%d/%s_test.pdf", SN, argCH, root_tree);
  canv->Print("temp/testPlot.pdf");

  //file->cd(); file->Write();
//  char namecyc[20];
//  sprintf(namecyc, "%s;*", root_tree);
//  gDirectory->Delete(namecyc);
  delete canv;
  file->Close();
}
