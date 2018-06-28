#ifndef extract_edges_h__
#define extract_edges_h__
#include "Feature_extration/feature.hh"
#include <vector>

inline feature extract_rising_edge(const std::vector<int>& ADC_counts, const int thr, const int thr_min) {
	feature edge;
	bool seen_min = false;
	for (size_t i = 0; i < ADC_counts.size(); i++)
	{
		if (ADC_counts[i] < thr_min) {
			seen_min = true;
		}
		if (ADC_counts[i] > thr) {
			edge.time = ((double)thr - ADC_counts[i - 1]) / (double)(ADC_counts[i] - ADC_counts[i - 1]) + (i - 1);
			edge.signal = (double)(ADC_counts[i] - ADC_counts[i - 1]) *(edge.time - (i - 1)) + ADC_counts[i - 1];
			break;
		}

	}

	return edge;
}

inline feature extract_faling_edge(const std::vector<int>& ADC_counts, const int thr, const int thr_max) {
	feature edge;
	bool seen_max = false;

	for (size_t i = 0; i < ADC_counts.size(); i++)
	{
		if (ADC_counts[i] > thr_max) {
			seen_max = true;
		}
		if (ADC_counts[i] < thr && seen_max) {
			edge.time = ((double)thr - ADC_counts[i - 1]) / (double)(ADC_counts[i] - ADC_counts[i - 1]) + (i - 1);
			edge.signal = (double)(ADC_counts[i] - ADC_counts[i - 1]) *(edge.time - (i - 1)) + ADC_counts[i - 1];
			break;
		}

	}

	return edge;
}

inline feature extract_time_over_threshold(const std::vector<int>& ADC_counts, const int thr, const int thr_max) {
	auto rising = extract_rising_edge(ADC_counts, thr, 1e9);
	auto falling = extract_faling_edge(ADC_counts, thr, thr_max);
	feature edge;
	edge.signal = falling.time - rising.time;
	edge.time = (falling.time + rising.time) / 2;
	return edge;
}

#endif // extract_edges_h__
