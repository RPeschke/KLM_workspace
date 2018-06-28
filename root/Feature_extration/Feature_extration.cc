#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>


#include "TGraph.h"

#include "Feature_extration/extract_peak.hh"
#include "Feature_extration/helper_int_vector.hh"
#include "Feature_extration/filter_waveform.hh"
#include "Feature_extration/extract_area.hh"
#include "Feature_extration/extract_edges.hh"





class KLM_Tree;
KLM_Tree* Feature_extration(){

    gInterpreter->ProcessLine(".L Feature_extration/KLM_Tree.cc");
    TFile *_file0 = TFile::Open("C:/Users/Argg/Documents/Github/KLM/data/KLMS_0173.root");
	auto tree = dynamic_cast<TTree*>(_file0->Get("tree"));

    auto t1 = new  KLM_Tree(tree);


	auto fout = new TFile("test.root", "recreate");
    
	auto out_tree = new TTree("features","features");
	feature_branche branch_peak(out_tree, "peak");
	feature_branche branch_falling_edge(out_tree, "falling");
    feature_branche branch_rising_edge(out_tree, "rising");
    feature_branche branch_TOT(out_tree, "TOT");
	for (int i = 0; i < tree->GetEntries(); ++i) {


		t1->GetEntry(i);
		auto vec = to_vector(t1->ADC_counts[0]);
		auto vec1 = filter_waveform(vec, 100);
		branch_peak << extract_peak(vec1);
		branch_falling_edge << extract_faling_edge(vec1, 202, 250);

		branch_rising_edge << extract_rising_edge(vec1, 202, 100);
		

		

		branch_TOT << extract_time_over_threshold(vec1, 200, 250);
        
        
        out_tree->Fill();
	}
 //   out_tree->Write();
    fout->Write();
    return t1;
}