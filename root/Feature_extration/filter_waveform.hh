#ifndef filter_waveform_h__
#define filter_waveform_h__

#include <vector>
#include "TMath.h"


inline std::vector<int> filter_waveform(const std::vector<int>& ADC_counts, const int cut_off) {

	std::vector<int> ret = ADC_counts;


	for (size_t i = 1; i < ADC_counts.size() - 1; i++)
	{
		int mean_ = (ADC_counts[i - 1] + ADC_counts[i + 1]) / 2;
		if (TMath::Abs(mean_ - ADC_counts[i]) > cut_off) {
			ret[i] = mean_;
		}



	}
	ret[0] = ret[2];
	ret[ret.size() - 1] = ret[ret.size() - 3];
	return ret;
}


#endif // filter_waveform_h__