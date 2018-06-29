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

#include "Feature_extration/Feature_extraction.hh"




void Feature_extration(){


	Feature_extraction_read_file("C:/Users/Argg/Documents/Github/KLM/data/KLMS_0173.root", "tree");
  
}