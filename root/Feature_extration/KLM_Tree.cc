#define KLM_Tree_cxx
#include "KLM_Tree.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void KLM_Tree::Loop(Long64_t begin_ , Long64_t end_ )
{
//   In a ROOT session, you can do:
//      Root > .L KLM_Tree.C
//      Root > KLM_Tree t
//      Root > t.GetEntry(12); // Fill t data members with entry number 12
//      Root > t.Show();       // Show values of entry 12
//      Root > t.Show(16);     // Read and show values of entry 16
//      Root > t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
TH2D* h2 = new TH2D("","",257,-128,128,1000,-500,700);
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      for(int j =begin_ ; j < end_ ;++j){
        for(int adc = 0 ; adc<128;++adc){
          h2->Fill( (Float_t) adc - Time[j] ,ADC_counts[j][adc]);
          
        }
      }
   }
   h2->Draw("COLZ");
   
}

TH2D*   KLM_Tree::Draw_event(Long64_t jentry){

  TH2D* h2 = new TH2D("","",100,0,128,1000,-500,700);

   Long64_t nbytes = 0, nb = 0;

      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) return nullptr;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      for(int j =0 ; j < 16 ;++j){
        for(int adc = 0 ; adc<128;++adc){
          h2->Fill(adc,ADC_counts[j][adc]);
          
        }
      }
   
   h2->Draw("COLZ");
  return h2;
}
TGraph*   KLM_Tree::Draw_event(Long64_t jentry,Long64_t j){

  TGraph* h2 = new TGraph();

   Long64_t nbytes = 0, nb = 0;

      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) return nullptr;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      
        for(int adc = 0 ; adc<128;++adc){
          h2->SetPoint(adc, (Float_t) adc - Time[j] ,ADC_counts[j][adc]);
          
        }
      
   
   h2->Draw("A*");
  return h2;
}