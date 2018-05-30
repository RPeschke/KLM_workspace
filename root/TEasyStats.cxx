// TEasyStats.cxx
// author: Chris Ketter
// 11/4/2017

TClass TEasyStats {
  public:
  int    EvtNum;
  int    AddNum;
  int    WrAddNum;
  int    Wctime; // ???
  int    ASIC;
  int    Sample[16][128];
  int    PeakVal[16];
  int    PeakTime[16];
  // note: Window information is only allocated 3 bits, so
  //       window number is fully specified by sample number
  TEasyStats() {}  // Constructor
  ~TEasyStats() {} // Destructor
  TEasyStats(int n=1, int phase=45, TH1F* h1) {}
  TEasyStats(int n=2, int phase=45, TH1F* f1, TH1F* f2) {}
  TEasyStats(int n=3, int phase=45, TH1F* f1, TH1F* f2, TH1F* f3) {}
  TEasyStats(int n=4, int phase=45, TH1F* f1, TH1F* f2, TH1F* f3, TH1F* f4) {}
  TEasyStats(int n=5, int phase=45, TH1F* f1, TH1F* f2, TH1F* f3, TH1F* f4, TH1F* f5) {}

  gStyle->SetOptStat(0);// can use "e" for entries, but adds to every instance of TPaveStats
  gStyle->SetOptFit();// note, can't add arguments here without screwing up text size
  gStyle->SetStatY(.93);
  gStyle->SetStatX(.95);
  gStyle->SetStatW(0.15);
  float height = 0.1;
  float width = 0.1;
  gStyle->SetStatH(height);

  TPaveStats *stat1 = (TPaveStats*)(f1->FindObject("stats"));
  TPaveStats *stat2 = (TPaveStats*)(f2->FindObject("stats"));
  TPaveStats *stat3 = (TPaveStats*)(f3->FindObject("stats"));
  TPaveStats *stat4 = (TPaveStats*)(f4->FindObject("stats"));
  TPaveStats *stat5 = (TPaveStats*)(f5->FindObject("stats"));

  Int_t numX = (Int_t)((n-1)*TMath::Cos(phase)*TMath::Cos(phase));
  Int_t numY = n - numX - 1;
  Bool_t signX = (Bool_t)(1+TMath::Cos(phase)/(2*TMath::Abs(TMath::Cos(phase))));
  Bool_t signY = (Bool_t)(1+TMath::Sin(phase)/(2*TMath::Abs(TMath::Sin(phase))));

  Float_t x1[n]={0};
  if(signX){
    for (int i=0; i<numX; i++){
      x1[i]=(0.8-(numX-i)*0.2);
    }
    for (int i=numX; i<n; i++){
      x1[i]=0.8;
    }
  }
  else {
    for (int i=0; i<(n-numX); i++){
      x1[i]=0.0;
    }
    for (int i=(n-numX); i<n; i++){
      x1[i]=(i-n-numX+1)*0.2;
    }
  }
  if(signY){
    for (int i=0; i<numY; i++){
      y1[i]=(0.8-i*0.2);
    }
    for (int i=numY; i<n; i++){
      y1[i]=0.8;
    }
  }
  else {
    for (int i=0; i<(n-numY); i++){
      y1[i]=0.2*i;
    }
    for (int i=(n-numY); i<n; i++){
      y1[i]=0.2;
    }
  }

    stat0->SetTextColor(2);
    x1[0] = (1.0 + TMath::Cos(phase-45))/2.0;
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


};









}
