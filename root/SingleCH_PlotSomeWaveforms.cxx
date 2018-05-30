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
void PlotSomeWaveforms(const char* SN,
                       const char* root_dir,
                       const char* root_tree,
                       const int   numPages,
                       const int   argCH     ){
  gROOT->Reset();
  gStyle->SetOptStat(0);

 // declare tree variables
  Int_t EvtNum, AddNum;
  Int_t PeakTime, t1_1PE, t2_1PE;
  Short_t Sample[128];
  Float_t FIRsample[128], dFIRsample[127], PeakVal;

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
  //tree->SetBranchAddress("EvtNum", &EvtNum);
  tree->SetBranchAddress("AddNum", &AddNum);
  //tree->SetBranchAddress("WrAddNum", &WrAddNum);
  //tree->SetBranchAddress("Wctime", &Wctime);
  //tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("PeakTime_ns", &PeakTime);
  tree->SetBranchAddress("PeakVal_mV", &PeakVal);
  //tree->SetBranchAddress("partialQ_fC", &partialQ);
  tree->SetBranchAddress("TimeStart_ns", &t1_1PE);
  tree->SetBranchAddress("TimeStop_ns", &t2_1PE);
  tree->SetBranchAddress("Sample_mV", Sample);
  tree->SetBranchAddress("FIRsample_mV", FIRsample);
  tree->SetBranchAddress("dFIRsample_mV", dFIRsample);

  TCanvas* canv = new TCanvas("canv", "Test Canvas", 1100, 850);
  canv->Divide(4, 4); // make 4 pads per canvas


  //--- DECLARE 16 HISTOGRAMS & CONFIGURE ---//
  TH1I* hist[16]; TH1I* dhist[16];
  for (int i=0; i<16; i++){
    hist[i] = new TH1I("", "", 128,0,128);
    dhist[i] = new TH1I("", "", 128,0,128);
    hist[i]->GetYaxis()->SetTitleOffset(1.5);
    hist[i]->SetMaximum(75);
    hist[i]->SetMinimum(-25);
    hist[i]->SetMarkerStyle(7); //medium dot
    hist[i]->GetYaxis()->SetTitle("ADC Counts");
  }

  char xlabel[30];
  sprintf(xlabel, "Ch_%d Sample No.", argCH);
  char m_coord[12];
  char wlabel[20];
  int pad, plotNo = 0, pageNo=1;

  int center=8;
  int range=8;
  int numPlots=numPages*16;
  //--- LABEL, FILL, AND DRAW PLOTS ---//
  int numEnt = tree->GetEntriesFast();
  for(int e=0; e<numEnt; e++) {
    tree->GetEntry(e);
    if (PeakVal < (center-range) || PeakVal > (center+range)) continue;
    int pi = plotNo%16;//pad index
    //if (PeakTime[0]<96 && PeakTime[0]>10){
      //--- LABEL WINDOW AND CHANNEL ON PLOTS ---//
      sprintf(wlabel, "Windows %d-%d", AddNum, (AddNum+3)%512);
      hist[pi]->SetTitle(wlabel);
      hist[pi]->Reset();
      hist[pi]->GetXaxis()->SetTitle(xlabel);

      //--- LABEL PEAK VALUE ---//
      Double_t xpeak = (Double_t) PeakTime;
      Double_t ypeak = (Double_t) PeakVal;
      TMarker *m = new TMarker(xpeak, ypeak, 22);
      m->SetMarkerColorAlpha(kMagenta+3, 0.35);
      m->SetMarkerStyle(kPlus);
      m->SetMarkerSize(5);
      //hist[pi]->GetListOfFunctions()->Add(m);

      //--- DRAW PEAK COORDINATES ON PLOT ---//
      sprintf(m_coord, "[%3.0f, %3.0f]", xpeak, ypeak);
      TPaveLabel *l = new TPaveLabel(xpeak+3,ypeak+25,xpeak+23,ypeak+75, m_coord);
      //hist[pi]->GetListOfFunctions()->Add(l);

      for(int j=0; j<128; j++) { // sample No.
        hist[pi]->SetBinContent(j,Sample[j]);
        if (j<127) dhist[pi]->SetBinContent(j,dFIRsample[j]);
      }
      canv->cd(pi+1);
      hist[pi]->Draw("p");
      //dhist[pi]->Draw("SAMES");
      hist[pi]->SetMarkerStyle(6); //medium dot
      hist[pi]->SetMarkerColor(kAzure-6); //medium dot
      canv->Update();



      //--- SAVE PLOTS AS PDF ---//
      char plotName[100];
      if (plotNo==15 && numPages>1){
        sprintf(plotName, "data/%s/plots/ch%d/%dwaveforms_peak_within_%d_of_%d.pdf(", SN, argCH, numPlots, range, center);
        canv->Print(plotName); // 1st page
        pageNo++;
        plotNo++;
        continue;
      }
      if (  ( (numPages>2)  &&   (plotNo>15)  &&  ((plotNo%16)==15)  &&  pageNo!=numPages )  |  (plotNo==15 && numPages==1)  ) {
        sprintf(plotName, "data/%s/plots/ch%d/%dwaveforms_peak_within_%d_of_%d.pdf", SN, argCH, numPlots, range, center);
        canv->Print(plotName); // intermediate pages
        canv->Clear();
        canv->Divide(4,4);
        pageNo++;
        plotNo++;
        if (numPages==1) break;
        continue;
      }
      if ( ((plotNo%16)==15)  &&  pageNo==numPages) {  // last page
        sprintf(plotName, "data/%s/plots/ch%d/%dwaveforms_peak_within_%d_of_%d.pdf)", SN, argCH, numPlots, range, center);
        canv->Print(plotName);
        break;
      }
      plotNo++;
    //}
  }
  delete canv;
  for (int i=0; i<4; i++){delete hist[i];}
}
