void PlotPedestalStatisticsOneASIC(const char* root_file, const char* out_pdf) {
  gROOT->Reset();
  // declare tree variables
  Int_t EvtNum, AddNum,  WrAddNum;
//  Int_t Wctime, ASIC, PeakTime[16];
  Float_t PeakVal[16], Sample[16][128];


  TFile* file = new TFile(root_file,"READ");
  TTree* tree = (TTree*)file->Get("tree");
  TCanvas* canv = new TCanvas("canvas1", "Test Canvas", 1100, 850);
  canv->Divide(3,2); // make 4 pads per canvas

  tree->SetBranchAddress("AddNum", &AddNum);
//  tree->SetBranchAddress("ASIC", &ASIC);
  tree->SetBranchAddress("ADC_counts", Sample);
  tree->GetEntry(0);

//  char plotTitle[100];
//  sprintf(plotTitle, "ASIC %d, Ped. Compensated Baseline, All Samp/Ch Except Calib Ch", ASIC);
//  TH1I* hAll = new TH1I("hAll", plotTitle, 100, 3800, 4200);
//  hAll->GetXaxis()->SetTitle("Wilk. ADC Counts");
//
//  sprintf(plotTitle, "ASIC %d Calib Ch, Ped. Compensated Baseline", ASIC);
//  TH1I* hCalib = new TH1I("hCalib", plotTitle, 100, 1800, 2200);
//  hCalib->GetXaxis()->SetTitle("Wilk. ADC Counts");
//
//  TH3I* h[16];
//  for (int ch=0; ch<16; ch++){
//    h = new TH3I("hSAM", "PedSubdData",  512,0,512, 32,0,32, 4096,0,4096);
//  }
  h = new TH3I("hSAM", "PedSubdData",  512,0,512, 32,0,32, 4096,0,4096);
//  TH1I* h[16][512][32];
//  for (int ch=0; ch<16; ch++){
//    for (int win=0; win<512; win++){
//      for (int samp=0; samp<32; samp++;){
//        h[win][samp]= new TH1I("","",4096,0,4096);
//      }
//    }
//  }

  int NumEnt = tree->GetEntriesFast();
//  for (int e=0; e<NumEnt; e++){
  for (int e=0; e<100; e++){
    tree->GetEntry(e);
    for (int ch=0; ch<1; ch++){
      for (int grp=0; grp<4; grp++){
        for (int samp=0; samp<32; samp++){
          //h[AddNum+grp][samp]->Fill(Sample[ch][samp+32*grp]);
          //h->Fill(AddNum+grp,samp,Sample[ch][samp+32*grp]);
          (ch==15) ? h->Fill(AddNum+grp,samp,Sample[ch][samp+32*grp]-2050) : h->Fill(AddNum+grp,samp,Sample[ch][samp+32*grp]-3400);
          //(ch==15) ? hCalib->Fill(Sample[15][samp+32*grp]) : hAll->Fill(Sample[ch][samp+32*grp]);
        }
      }
    }
  }
//  canv->cd(1); hAll->Draw();
//  canv->cd(2); hCalib->Draw();
//  TH1I* hSamAll[32];
//  for (int samp=0; samp<32; samp++;){
//    hSamAll[samp] = new TH1I("", "", 4096,0,4096);
//  }
//
//  for (int ch=0; ch<16; ch++;){
//    for (int win=0; win<512; win++){
//      for (int samp=0; samp<32; samp++;){
//        hSamAll[samp].Add(h[win][samp])
//      }
//    }
//  }
//
//  sprintf(plotTitle, "ASIC %d Ped. Compensated Mean vs. Sample in All 512 Windows", ASIC);

  canv->cd(1); h->Project3D("x")->Draw("p");
  canv->cd(2); h->Project3D("y")->Draw("p");
  canv->cd(3); h->Project3D("z")->Draw("p");
  canv->cd(4); h->Project3D("xy")->Draw("COLZ");
  canv->cd(5); h->Project3D("yz")->Draw("COLZ");
  canv->cd(6); h->Project3D("zx")->Draw("COLZ");


//  hCalib->GetXaxis()->SetTitle("Wilk. ADC Counts");
//
//        = h[win][samp]->GetMean();
//        = h[win][samp]->GetMeanError();


  canv->Print(out_pdf);
}
