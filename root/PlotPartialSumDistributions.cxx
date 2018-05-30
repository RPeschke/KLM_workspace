void PlotPartialSumDistributions(const char* root_file, const char* root_dir, const char* root_tree, const int argASIC, const int argCH, const float argHV) {

  // I believe these have to be declared before first call to Draw(),
  // just leaving them at top for now.
  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  gStyle->SetStatH(height);

  Int_t AddNum, ASIC;
//  Int_t EvtNum, AddNum, WrAddNum, Wctime, ASIC;
  Int_t PeakTime, t1;
  Float_t PeakVal, partialQ;
//  Float_t RawSample[32];
//  Float_t Sample[16][128];
//  Float_t partialQ[16], totalQ[16], Qsquared[16];
//  Float_t AbsValue[16];
  Float_t mVperADC[10] = {.86094, .75229, .96909, .77299, .77299,
                          .77147, .82140, .75664, .86854, .84978};
  //Float_t mVperADC = 0.814765; // average of 10 ASICs from KLMS_0173
  Float_t fCper_mV = 20.0; //GSa/s / 50 Ohm
  Float_t HV = argHV;
  Int_t asic = argASIC;


  TFile* file = new TFile(root_file,"UPDATE");
  if (!gFile->Get(root_dir)) {
    printf("Can't find directory %s in %s . . . exiting!\n", root_dir, root_file);
    exit(-1);
  }
  file->cd(root_dir);
  if (!gDirectory->Get(root_tree)){
    printf("Can't find TTree %s in %s . . . exiting!\n", root_tree, root_dir);
    exit(-1);
  }
  TTree* tree = (TTree*)gDirectory->Get(root_tree);
//  tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
//  tree->SetBranchAddress("WrAddNum", &WrAddNum);
//  tree->SetBranchAddress("Wctime", &Wctime);
  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  tree->SetBranchAddress("partialQ_fC", &partialQ);
//  tree->SetBranchAddress("totalQ_fC", totalQ);
//  tree->SetBranchAddress("Qsquared_fC2", Qsquared);
//  tree->SetBranchAddress("AbsValue_fC", AbsValue);
//  tree->SetBranchAddress("TimeOverThresh", ToverTh);
  tree->SetBranchAddress("TimeStart_ns", &t1);
//  tree->SetBranchAddress("Sample_mV", Sample);


  // Declare memory on heap for canvas & configure
  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  canv->SetLogy();
//  canv->Divide(2,2);

  // Declare memory on heap for histogram & configure
  TH1F* hist0 = new TH1F("hist0", "Peak Value Distribution", 300,0,350);
  hist0->GetXaxis()->SetTitle("ADC counts");
  hist0->SetLineColor(13);
//
//  TH1F* hist1 = new TH1F("hist1", "Riemann Sum", 300,-500,7000);
//  hist1->GetXaxis()->SetTitle("Integrated ADC counts");
//  hist1->SetLineColor(13);

  TH1F* hist2 = new TH1F("hist2", "Partial Riemann Sum", 300,0,6000);
  hist2->GetXaxis()->SetTitle("Integrated ADC counts");
  hist2->SetLineColor(13);

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
    if (partialQ != 0.0) hist2->Fill(partialQ);
//    hist3->Fill(RiemannSumSq[argCH]);
//    hist4->Fill(AbsValue[argCH]);
//    hist5->Fill(ToverTh[argCH]);
//    hist6->Fill(ToverTh[argCH]*PeakVal[argCH]);
  }

  hist0->Draw();
  char plotName[50];
  sprintf(plotName, "data/KLMS_0173a/plots/ASIC%d_ch%d_HV%.2f_peakValdist.pdf", argASIC, argCH, argHV);
  canv->Print(plotName);
  canv->Clear();

  // Clone oritinal histogram for subsequent fit functions
  TH1F* clone1 = (TH1F*)(hist2->Clone());
  TH1F* clone2 = (TH1F*)(hist2->Clone());
  TH1F* clone3 = (TH1F*)(hist2->Clone());
  TH1F* clone4 = (TH1F*)(hist2->Clone());

  float width = 200.;
  Double_t Mean[5], dMean[5], Width[5], dWidth[5];
  float exp1PE = HV*4.81853e+02-3.37660e+04;
//  float exp2PE = HV*9.78835e+02-6.84374e+04;
//  float exp3PE = HV*1.54434e+03-1.07921e+05;
//  float exp4PE = HV*2.12429e+03-1.48415e+05;
//  float exp5PE = HV*2.46639e+03-1.71947e+05;

// 1PE
  TF1 *gaus1PE_i = new TF1("gaus1PE_i", "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (2*215*215))",exp1PE-400,exp1PE+400);
  gaus1PE_i->SetParameters(1250, exp1PE);
  gaus1PE_i->SetLineColor(2);
  hist2->Fit("gaus1PE_i", "RS"); // "R" for fit range
  Mean[0] = gaus1PE_i->GetParameter(1);
  hist2->Draw();
  TF1 *gaus1PE = new TF1("gaus1PE", "gaus", (Mean[0]-width), (Mean[0]+width));
  gaus1PE->SetLineColor(2);
  hist2->Fit("gaus1PE", "RS"); // "R" for fit range
  Mean[0]  = gaus1PE->GetParameter(1); dMean[0]  = gaus1PE->GetParError(1);
  Width[0] = gaus1PE->GetParameter(2); dWidth[0] = gaus1PE->GetParError(2);
  hist2->Draw();
  float onePE = Mean[0];

//  2PE
  char eqnf1[100];
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (4*%f*%f))", Width[0], Width[0]);
  TF1 *gaus2PE_i = new TF1("gaus2PE_i", eqnf1, (1.5*onePE), (2.5*onePE));
  gaus2PE_i->SetParameters(350, (2*onePE));
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


  onePE = 0.25*Mean[1] + 0.5*Mean[0];


// 3PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (6*%f*%f))", Width[0], Width[0]);
  TF1 *gaus3PE_i = new TF1("gaus3PE_i", eqnf1, (2.5*onePE), (3.5*onePE));
  gaus3PE_i->SetParameters(95, 2570);
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

// 4PE
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (8*%f*%f))", Width[0], Width[0]);
  TF1 *gaus4PE_i = new TF1("gaus4PE_i", eqnf1, (3.5*onePE), (4.5*onePE));
  gaus4PE_i->SetParameters(25, 3425);
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
  sprintf(eqnf1, "[0]*TMath::Exp( -TMath::Power( (x-[1]), 2) / (10*%f*%f))", Width[0], Width[0]);
  TF1 *gaus5PE_i = new TF1("gaus5PE_i", eqnf1, (4.5*onePE), (5.5*onePE));
  gaus5PE_i->SetParameters(6, 4344);
  gaus5PE_i->SetLineColor(38);
  clone4->Fit("gaus5PE_i", "RS", "SAMES");
  Mean[4] = gaus5PE_i->GetParameter(1); dMean[4] = gaus5PE_i->GetParError(1);
  clone4->Draw("SAMES");
  TF1 *gaus5PE = new TF1("gaus5PE", "gaus", (Mean[4]-width), (Mean[4]+width));
  gaus5PE->SetParameters(gaus5PE_i->GetParameter(0), Mean[4],400);
  gaus5PE->SetLineColor(38);
  clone4->Fit("gaus5PE", "RS", "SAMES");
//  Mean[4] = gaus5PE->GetParameter(1); dMean[4] = gaus5PE->GetParError(1);
  Width[4] = gaus5PE->GetParameter(2); dWidth[4] = gaus5PE->GetParError(2);
  clone4->Draw("SAMES");

  // Modify Stat boxes to include ALL fit results
  gPad->Update();// N.B. This line is a MUST or else the following TPaveStats
                 // pointers will return as "null." If using TCanvas::Divide()
                 // to make multiple pads, then "gPad->Update()" should be used.
                 // See TPaveStats class reference for more details.
  TPaveStats *stat0 = (TPaveStats*)(hist2->FindObject("stats"));
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

  canv->Write("PEdist");
  sprintf(plotName, "data/KLMS_0173a/plots/ASIC%d_ch%d_HV%.2f_PEdist.pdf", argASIC, argCH, argHV);
  canv->Print(plotName);

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

  //file->cd(); file->Write();
//  char namecyc[20];
//  sprintf(namecyc, "%s;*", root_tree);
//  gDirectory->Delete(namecyc);
  delete canv;
  file->Close();
}
