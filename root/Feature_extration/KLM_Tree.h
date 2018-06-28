//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sun May 27 14:39:39 2018 by ROOT version 5.34/18
// from TTree tree/SciFi tracker output via KLM motherboard
//////////////////////////////////////////////////////////

#ifndef KLM_Tree_h
#define KLM_Tree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TH2.h"
#include "TGraph.h"

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class KLM_Tree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           EvtNum;
   Int_t           AddNum;
   Int_t           WrAddNum;
   Int_t           Wctime;
   Int_t           ASIC;
   Int_t           ADC_counts[16][128];
   Bool_t          FeatExtActivated[16];
   Int_t           PeakVal[16];
   Int_t           NumSampsInTopEighth[16];
   Float_t         Time[16];
   Bool_t          AfterPulseFlag[16];
   Int_t           AfterPulseRelativeAmplitude[16];
   Float_t         TimeOverThr[16];

   // List of branches
   TBranch        *b_EvtNum;   //!
   TBranch        *b_AddNum;   //!
   TBranch        *b_WrAddNum;   //!
   TBranch        *b_Wctime;   //!
   TBranch        *b_ASIC;   //!
   TBranch        *b_ADC_counts;   //!
   TBranch        *b_FeatExtActivated;   //!
   TBranch        *b_PeakVal;   //!
   TBranch        *b_NumSampsInTopEighth;   //!
   TBranch        *b_Time;   //!
   TBranch        *b_AfterPulseFlag;   //!
   TBranch        *b_AfterPulseRelativeAmplitude;   //!
   TBranch        *b_TimeOverThr;   //!

   KLM_Tree(TTree *tree=0);
   virtual ~KLM_Tree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Long64_t begin_= 0 , Long64_t end_ =16);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual TH2D*    Draw_event(Long64_t entry);
   virtual TGraph*    Draw_event(Long64_t jentry,Long64_t j);
};

#endif

#ifdef KLM_Tree_cxx
KLM_Tree::KLM_Tree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("C:/Users/Argg/Documents/Github/KLM/data/KLMS_0173.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("C:/Users/Argg/Documents/Github/KLM/data/KLMS_0173.root");
      }
      f->GetObject("tree",tree);

   }
   Init(tree);
}

KLM_Tree::~KLM_Tree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t KLM_Tree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t KLM_Tree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void KLM_Tree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EvtNum", &EvtNum, &b_EvtNum);
   fChain->SetBranchAddress("AddNum", &AddNum, &b_AddNum);
   fChain->SetBranchAddress("WrAddNum", &WrAddNum, &b_WrAddNum);
   fChain->SetBranchAddress("Wctime", &Wctime, &b_Wctime);
   fChain->SetBranchAddress("ASIC", &ASIC, &b_ASIC);
   fChain->SetBranchAddress("ADC_counts", ADC_counts, &b_ADC_counts);
   fChain->SetBranchAddress("FeatExtActivated", FeatExtActivated, &b_FeatExtActivated);
   fChain->SetBranchAddress("PeakVal", PeakVal, &b_PeakVal);
   fChain->SetBranchAddress("NumSampsInTopEighth", NumSampsInTopEighth, &b_NumSampsInTopEighth);
   fChain->SetBranchAddress("Time", Time, &b_Time);
   fChain->SetBranchAddress("AfterPulseFlag", AfterPulseFlag, &b_AfterPulseFlag);
   fChain->SetBranchAddress("AfterPulseRelativeAmplitude", AfterPulseRelativeAmplitude, &b_AfterPulseRelativeAmplitude);
   fChain->SetBranchAddress("TimeOverThr", TimeOverThr, &b_TimeOverThr);
   Notify();
}

Bool_t KLM_Tree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void KLM_Tree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t KLM_Tree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef KLM_Tree_cxx
