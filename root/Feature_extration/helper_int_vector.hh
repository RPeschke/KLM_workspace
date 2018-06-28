#ifndef helper_int_vector_h__
#define helper_int_vector_h__
#include "TGraph.h"
#include <string>
#include <vector>



inline TGraph*  Draw(const std::vector<int>& ADC_counts,const std::string& option = "A*") {
	TGraph* ret = new TGraph();

	for (size_t i = 0; i < ADC_counts.size(); i++) {

		ret->SetPoint(i, i, ADC_counts[i]);

	}
	ret->Draw(option.c_str());
	return ret;
}


inline std::vector<int> to_vector(int* ADC_counts, size_t ADC_counts_size = 128){
    std::vector<int> ret;
    ret.reserve(ADC_counts_size );
    
    
    for(size_t i = 0; i < ADC_counts_size; i++)
    {
        ret.push_back(ADC_counts[i]);
    }
    

    return ret;

}
#endif // helper_int_vector_h__
