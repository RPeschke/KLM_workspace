#ifndef feature_h__
#define feature_h__
#include <string>
#include <iostream>
#include "TGraph.h"
#include "TTree.h"

struct feature {
	double time = -100, signal = -100;

};

inline std::ostream& operator<<(std::ostream& out, const feature& f) {
	out << "time: " << f.time << " Signal: " << f.signal;
	return out;
}

inline TGraph* Draw(const feature& f,std::string options = "*") {
	TGraph* ret = new TGraph();
	ret->SetPoint(ret->GetN(), f.time, f.signal);
	ret->SetMarkerColor(4);
	ret->Draw(options.c_str());

}

class feature_branche {
public:
	inline feature_branche(TTree* out_tree,const std::string& name) {
		m_feature = new feature();
		out_tree->Branch((name + "_signal").c_str(), &m_feature->signal);
		out_tree->Branch((name + "_time").c_str(), &m_feature->time);
	}
	inline ~feature_branche() {
		delete m_feature;
	}


	friend inline feature_branche& operator<<(feature_branche& out, const feature& f) {
		if(f.signal < 2000){
			out.m_feature->signal = f.signal;
		}else {
			out.m_feature->signal =-10;
		}
		if(f.time < 2000){
			out.m_feature->time = f.time;
		}else {
			out.m_feature->time = -10;
		}

		
		return out;
	}
private:
	feature* m_feature;
};

#endif // feature_h__
