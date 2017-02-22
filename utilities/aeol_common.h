#include <map>
#include <unordered_map>
using namespace std;

/* final RT */
map<uint64_t, double> RT;

/* final FP */
map<uint64_t, double> FP;

/* final MR */
map<uint64_t, double> MR;


void RTtoMR_AET() {

	//cout << "Start convert RT to MR" << endl;

	std::map<uint64_t, double> P;
	double total_num_RT = 0;
	uint64_t max_RT = 0;

	for (std::map<uint64_t, double>::reverse_iterator it = RT.rbegin(), eit = RT.rend(); it != eit; ++it) {
		total_num_RT += it->second;
		if (max_RT < it->first) {
			max_RT = it->first;
		}
	}
	double accumulate_num_RT = 0;
	for (std::map<uint64_t, double>::reverse_iterator it = RT.rbegin(), eit = RT.rend(); it != eit; ++it) {
		P[it->first] = accumulate_num_RT / total_num_RT;
		//cout << accumulate_num_RT << " " << total_num_RT << endl;
		accumulate_num_RT += it->second;
		//cout << "P " << it->first << " " << P[it->first] << endl;
	}

	P[0] = 1;

	double sum_P = 0;
	uint64_t t = 0;
	uint64_t prev_t = 0;
	for (uint64_t c = 0; c <= max_RT; c++) {
		while (sum_P < c && t <= max_RT) {
			if (P.find(t) != P.end()) {
				sum_P += P[t];
				prev_t = t;
			} else {
				sum_P += P[prev_t];
			}
			t++;
		}
		/*
		if (P.find(t) != P.end()) {
			MR[c] = P[t];
		} else {
			MR[c] = P[prev_t];
		}*/
		MR[c] = P[prev_t];
	}

	return;
}

void reset_common() {
	MR.clear();
	RT.clear();
	FP.clear();
	return;
}


void dumpMR() {
	
	cout << "miss ratio" << endl;
	
	auto it1 = MR.begin();
	auto it2 = MR.begin();

	while(it1 != MR.end()) {
		while(1) {
			auto it3 = it2;
			++it3;
			if (it3 == MR.end()) {
				break;
			}
			if (it3->second == it2->second) {
				++it2;
			} else {
				break;
			}
		}
		cout << it1->first << " " << it1->second << endl;
		if (it1 != it2) {
			cout << it2->first << " " << it2->second << endl;
		}
		it1 = ++it2;
		it2 = it1;
	}

	return;
}

void dumpFP() {

	cout << "footprint" << endl;

	auto it1 = FP.begin();
    auto it2 = FP.begin();	

	while(it1 != FP.end()) {
		while(1) {
            auto it3 = it2;
			++it3;
            if (it3 == FP.end()) {
                break;
            }
            if (it3->second == it2->second) {
                ++it2;
            } else {
                break;
            }
        }
    
		cout << it1->first << " " << it1->second << endl;
    	if (it1 != it2) {
        	cout << it2->first << " " << it2->second << endl;
    	}
    	it1 = ++it2;
    	it2 = it1;
	}

    return;
}


void dumpRT() {

    for (std::map<uint64_t, double>::iterator it = RT.begin(), eit = RT.end(); it != eit; ++it) {
        cout << it->first << " " << it->second << endl;
    }

    return;
}

void dumpALL() {
	dumpFP();
	dumpMR();
	return;
}

void dumpNone() {
	return;
}
