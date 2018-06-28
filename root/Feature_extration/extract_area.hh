#ifndef extract_area_h__
#define extract_area_h__


#include "Feature_extration/feature.hh"

#include <vector>

inline feature extract_area(const std::vector<int>& ADC_counts, const int thr) {
	feature area;
	bool first = true;

	for (size_t i = 0; i < ADC_counts.size(); i++)
	{
		if (ADC_counts[i] > thr) {

			area.signal += ADC_counts[i] - thr;
			if (first) {
				// std::cout << "i: " << i << "  ADC_counts[i-1]: "<<ADC_counts[i-1] << "   ADC_counts[i]:  "<< ADC_counts[i]<< std::endl;
				area.time = ((double)thr - ADC_counts[i - 1]) / (double)(ADC_counts[i] - ADC_counts[i - 1]) + (i - 1);
				first = false;
			}


		}

	}

	return area;

}


#endif // extract_area_h__
