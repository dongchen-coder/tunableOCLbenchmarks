#include <set>
#include <map>
#include <stdio.h>
using namespace std;

std::map<uint64_t, uint64_t> L;
std::map<uint64_t, uint64_t> avgL;
std::set<uint64_t> D_w;
std::map<uint64_t, std::set<uint64_t>* > DtoW;

uint64_t wgid_s = 0;
uint64_t trace_len = 0;
uint64_t ref_time = 0;
uint64_t N = 0;

void accessErrL(uint64_t addr, uint64_t wgid) {

	ref_time++;
	trace_len++;

	if (wgid != wgid_s) {
		for (std::set<uint64_t>::iterator it = D_w.begin(), eit = D_w.end(); it != eit; ++it) {
			if (avgL.find(*it) != avgL.end()) {
				avgL[*it] += ref_time - 1;
			} else {
				avgL[*it] = ref_time - 1;
			}
			
			if (DtoW.find(*it) != DtoW.end()) {
				DtoW[*it]->insert(wgid-1);
			} else {
				DtoW[*it] = new std::set<uint64_t>;
				DtoW[*it]->insert(wgid-1);
			}
		}
	
		L[wgid-1] = ref_time - 1;

		N++;
		ref_time = 1;
		D_w.clear();
		D_w.insert(addr);

	} else {
		D_w.insert(addr);
	}

	wgid_s = wgid;
	
	return;
}

double averagedCount(int n, int m, int i) {
    //printf("Caculate n m i %d %d %d\n", n, m, i);
    double res = 1;
    for (int j = 0; j < i; j++) {
        res *= double (m-j) / (n-j);
    }
    return res / (n-i);
}


void calculateErrL() {
	
	for (std::set<uint64_t>::iterator it = D_w.begin(), eit = D_w.end(); it != eit; ++it) { 
		if (avgL.find(*it) != avgL.end()) {
			avgL[*it] += ref_time;
		} else {
			avgL[*it] = ref_time;
		}

		if (DtoW.find(*it) != DtoW.end()) {
			DtoW[*it]->insert(wgid_s);
		} else {
			DtoW[*it] = new std::set<uint64_t>;
			DtoW[*it]->insert(wgid_s);
		}
	}

	N++;	
	L[wgid_s] = ref_time;

	double totalCount = 0;	

	for (std::map<uint64_t, uint64_t>::iterator it = avgL.begin(), eit = avgL.end(); it != eit; ++it) {
		if (N - DtoW[it->first]->size() != 0) {
			avgL[it->first] = (trace_len - it->second) / (N - DtoW[it->first]->size());
		} else {
			DtoW.erase(it->first);
			totalCount += N-1;
		}
	}

	double errorP = 0.0;

	for (std::map<uint64_t, std::set<uint64_t>* >::iterator it = DtoW.begin(), eit = DtoW.end(); it != eit; ++it) {
	 	printf("Data %llu\n", it->first);
		uint64_t errC = 0;
		for (std::map<uint64_t, uint64_t>::iterator lit = L.begin(), elit = L.end(); lit != elit; ++lit) {
			if (it->second->find(lit->first) == it->second->end()) {
				uint64_t error;
				if (lit->second > avgL[it->first]) {
					error = lit->second - avgL[it->first];
				} else {
					error = avgL[it->first] - lit->second;
				}
				//printf("    err %llu\n", error);
				if (error > 0) {
					errC++;
				}
			}
		}

		double tmpC = errC;
		double tmpM = N-DtoW[it->first]->size();
		
		for (int i = 0; i < N-DtoW[it->first]->size(); i++) {
			tmpC *= errC-i;
			tmpM *= (N-DtoW[it->first]->size() - i);
			double tmp = averagedCount(N, N-DtoW[it->first]->size(), i);
			errorP += tmp * tmpC / tmpM;
			totalCount += tmp;
		}
	}

	errorP = errorP / totalCount; 

	printf("The percentage of the rt that have error: %f\n", errorP);

	return;
}

void resetErrL() {
	
	L.clear();
	avgL.clear();
	D_w.clear();
	DtoW.clear();

	wgid_s = 0;
	trace_len = 0;
	ref_time = 0;
	N = 0;

	return;
}

