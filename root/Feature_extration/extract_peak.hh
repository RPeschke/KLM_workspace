#ifndef extract_peak_h__
#define extract_peak_h__
#include "Feature_extration/feature.hh"
#include <vector>


inline feature extract_peak(const std::vector<int>& ADC_counts) {


	feature max_element;


	for (size_t i = 0; i < ADC_counts.size(); i++)
	{
		if (ADC_counts[i] > max_element.signal) {

			max_element.signal = ADC_counts[i];
			max_element.time = i;

		}

	}

	return max_element;
}




#endif // extract_peak_h__
