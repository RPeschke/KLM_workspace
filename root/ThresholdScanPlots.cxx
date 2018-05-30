////////////////////////////////////////////////////////////////////////////////
//               Plot distribution & write results in new TFile               //
////////////////////////////////////////////////////////////////////////////////
void Find_xIntercept(const char* root_file, const char* plotTitle, int trigThresh, int ch, float EvtRate, int ThIndex){
const char* root_file, const char* root_dir, const char* root_tree) {
using namespace TMath;

  Float_t RiemannSum[16];

  TFile* file0 = new TFile(root_file,"READ");

  TTree* tree0 = (TTree*)file0->Get("tree");
  tree0->SetBranchAddress("RiemannSum", RiemannSum);

  // Declare memory on heap for canvas & configure
//  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1000, 1000);
  //canv->SetLogy();


  Double_t ctsPerBin = 5000./100;
  TH1I* hist = new TH1I("", "",100,0,5000);
  hist->SetTitle(plotTitle);
  hist->SetMaximum(1000);
  hist->GetXaxis()->SetTitle("Integrated ADC Counts / Evt");
  hist->GetXaxis()->SetLabelSize(.025);
  hist->GetYaxis()->SetTitle("# of Evts / bin (100 bins)");
  hist->GetYaxis()->SetLabelSize(.025);
  //hist->GetYaxis()->SetTitleOffset(1.1);
  // Fill Histogram
  int numEnt = tree0->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree0->GetEntry(e);
      hist->Fill(RiemannSum[ch]);
  }

  // fit middle 60% of leading edge to line and get x-intercept
  int peakBin;
  double peakVal = 0.0;
  double diff20, diff80, minDiff20, minDiff80, fitBeg, fitEnd;
  minDiff20 = minDiff80 = 20000.;
  for(int b=1; b<300; b++) {
    if (hist->GetBinContent(b) > peakVal){
      peakBin = b;
      peakVal = hist->GetBinContent(b);
    }
  }
  for(int b=0; b<peakBin; b++) {
    diff20 = Abs(hist->GetBinContent(b)-0.1*peakVal);
    diff80 = Abs(hist->GetBinContent(b)-0.9*peakVal);
    if (diff20 < minDiff20){
      minDiff20 = diff20;
      fitBeg = b*ctsPerBin;
    }
    if (diff80 < minDiff80){
      minDiff80 = diff80;
      fitEnd = b*ctsPerBin;
    }
  }
  //double p0, p1;
  TF1 *f1 = new TF1("f1", "pol1", fitBeg, fitEnd);
  f1->SetLineColor(kGreen);
  hist->Fit("f1", "R"); // "R" for fit range
  double xIntercept = -f1->GetParameter(0)/f1->GetParameter(1);
  double dxIntercpt = Sqrt(
                        Sq( f1->GetParError(0)/f1->GetParameter(1) )
                      + Sq( f1->GetParameter(0)*f1->GetParError(1)/Sq(f1->GetParameter(1)) )
                      );
  cout << "xIntercept: " << xIntercept << "\tdxIntercept: " << dxIntercpt << "\n";
  //hist->Draw();



  //char pdfOutfile[50];
  //sprintf(pdfOutfile, "outdir/thScan/Ch%d_Th_%d.pdf", ch, trigThresh);
  //canv->Print(pdfOutfile);
  //delete canv;


  //write fit results to new TTree
  TFile* file1 = new TFile("","UPDATE");
  TTree* tree1;
  if (file1->Get("tree")){
    tree1 = (TTree*)file1->Get("tree");
    tree1->SetBranchAddress("ThIndex", &ThIndex);
    tree1->SetBranchAddress("Channel", &ch);
    tree1->SetBranchAddress("EventRate", &EvtRate);
    tree1->SetBranchAddress("TrigThresh", &trigThresh);
    tree1->SetBranchAddress("xIntercept", &xIntercept);
    tree1->SetBranchAddress("dxIntercept", &dxIntercpt);
  }
  else {
    tree1 = new TTree("tree", "Trigger threshold and partial Riemann summ turn-on points");
    tree1->Branch("ThIndex", &ThIndex, "ThIndex/I");
    tree1->Branch("Channel", &ch, "Channel/I");
    tree1->Branch("EventRate", &EvtRate, "EvtRate/F");
    tree1->Branch("TrigThresh", &trigThresh, "TrigThresh/I");
    tree1->Branch("xIntercept", &xIntercept, "xIntercept/D");
    tree1->Branch("dxIntercept", &dxIntercpt, "dxIntercept/D");
  }
  tree1->Fill();
  tree1->Write("tree", TObject::kWriteDelete);
  char histTitle[30];
  sprintf(histTitle, "ch%d_th%d", ch, trigThresh);
  hist->Write(histTitle);
  file1->Close();
  file0->Close();
}


////////////////////////////////////////////////////////////////////////////////
//                        Plot Threshold Scan Results                         //
////////////////////////////////////////////////////////////////////////////////
void PlotThresholdScanResults(){

  TCanvas *c1 = new TCanvas("c1","", 1000, 1000);
  c1->SetGrid();
  c1->GetFrame()->SetBorderSize(12);

  TFile* file2 = new TFile("outdir/thScan/TrigTh_vs_IntegCts.root","UPDATE");
  TTree* tree2 = (TTree*)file2->Get("tree");


  Double_t xIntercept, dxIntercpt;
  Int_t ch, trigThresh, ThIndex;
  Float_t EvtRate;

  tree2->SetBranchAddress("ThIndex", &ThIndex);
  tree2->SetBranchAddress("Channel", &ch);
  tree2->SetBranchAddress("EventRate", &EvtRate);
  tree2->SetBranchAddress("TrigThresh", &trigThresh);
  tree2->SetBranchAddress("xIntercept", &xIntercept);
  tree2->SetBranchAddress("dxIntercept", &dxIntercpt);


  int numEnt = tree2->GetEntriesFast();
  Double_t xInt[15][11], dxInt[15][11], thr[15][11], evtR[15][11];

  for (int e=0; e<numEnt; e++){
    tree2->GetEntry(e);
    xInt[ch][ThIndex-1] = xIntercept;
    dxInt[ch][ThIndex-1] = dxIntercpt;
    thr[ch][ThIndex-1] = (double)trigThresh;
    evtR[ch][ThIndex-1] = (double)EvtRate;
    cout << thr[ch][ThIndex-1] << "\t";
  }

  TMultiGraph* mg1 = new TMultiGraph();
  TMultiGraph* mg2 = new TMultiGraph();

  TGraphErrors* gr1[15];
  TGraph* gr2[15];
  Double_t xIntCh[11], dxIntCh[11], thresh[11], evtRate[11];
  Char_t title1[20], title2[20];
  for (int i=0; i<15; i++){
    for (int j=0; j<11; j++){
      thresh[j] = thr[i][j];
      xIntCh[j] = xInt[i][j];
      dxIntCh[j] = dxInt[i][j];
      evtRate[j] = evtR[i][j];
    }
    sprintf(title1,"cutoffVsTh_ch%d",i);
    gr1[i] = new TGraphErrors(11, thresh, xIntCh, 0, dxIntCh);
    gr1[i]->SetTitle(title1); gr1[i]->SetMarkerColor(i); gr1[i]->SetMarkerStyle(20);
    gr1[i]->GetXaxis()->SetTitle("Trigger Threshold (DAC counts)");
    gr1[i]->GetYaxis()->SetTitle("Triggered Cutoff (integrated ADC counts)");
    mg1->Add(gr1[i]); gr1[i]->Write(title1);
    sprintf(title2, "freqVsTh_ch%d", i);
    gr2[i] = new TGraph(11, thresh, evtRate);
    gr2[i]->SetTitle(title2); gr2[i]->SetMarkerColor(i); gr2[i]->SetMarkerStyle(20);
    gr2[i]->GetXaxis()->SetTitle("Trigger Threshold (DAC counts)");
    gr2[i]->GetYaxis()->SetTitle("Triggered Cutoff (integrated ADC counts)");
    mg2->Add(gr2[i]); gr2[i]->Write(title2);
  }

  mg1->Draw("alp");//draw once before setting attributes
  mg1->SetTitle("Wilk. ADC cutoff");
  mg1->GetYaxis()->SetTitle("Triggered Cutoff (integrated ADC counts)");
  mg1->GetYaxis()->SetLabelSize(0.025);
  mg1->GetYaxis()->SetTitleOffset(1.25);
  mg1->GetXaxis()->SetTitle("Trigger Threshold (DAC counts)");
  mg1->GetXaxis()->SetLabelSize(0.025);
  mg1->SetMinimum(0);
  mg1->SetMaximum(2000);
  mg1->Draw("ap");

  mg2->Draw("ap");//draw once before setting attributes
  mg2->SetTitle("Freq. of Hits");
  mg2->GetYaxis()->SetTitle("Trigger freq (Hz)");
  mg2->GetYaxis()->SetLabelSize(0.025);
  mg2->GetYaxis()->SetTitleOffset(1.25);
  mg2->GetXaxis()->SetTitle("Trigger Threshold (DAC counts)");
  mg2->GetXaxis()->SetLabelSize(0.025);
  mg2->SetMinimum(0);
  mg2->SetMaximum(1000);
  mg2->Draw("alp");

  file2->cd();
  mg1->Write("ADCCutoffVsThresh");
  mg2->Write("TrigFreqVsThresh");

  file2->Close();
}
