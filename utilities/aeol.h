#include <stdlib.h>
#include <iostream>
#include <set>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>
#include "aeol_common.h"
using namespace std;

/* Cache line size */
#define CLS 32
/* DATA type size */
#define DTS 4

/* raw data */
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> > f;
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> > e;
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> > rt;
std::unordered_map<uint64_t, uint64_t> L;
std::unordered_map<uint64_t, uint64_t> prevs;
std::set<uint64_t> D;
std::unordered_map<uint64_t, double> interRT;

uint64_t ref_time = 0;
uint64_t trace_len = 0;

uint64_t N = 0;

/* processed data for alg 2 */
std::unordered_map<uint64_t, std::vector<uint64_t>* > F;
std::unordered_map<uint64_t, std::vector<uint64_t>* > E;
std::unordered_map<uint64_t, uint64_t> M;
std::unordered_map<uint64_t, uint64_t> aveL;

/* processed data for alg 3 */
uint64_t cid = 0;
std::unordered_map<std::string, uint64_t> CID;
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> > CEF;
std::unordered_map<uint64_t, uint64_t> CM;
std::unordered_map<uint64_t, uint64_t> CaveL;

/* intra RT*/
std::unordered_map<uint64_t, uint64_t> intraRT;

/* aeol F */
std::unordered_map<uint64_t, double> fRT;

/* aeol E */
std::unordered_map<uint64_t, double> eRT;


#define CLASSIFICATION

//#define ADDRESS_SAMPLING 
#define SAMPLING_RATE 100000

void address_sampling() {

	std::set<uint64_t> tmp;
	int i = 0;
	for (std::set<uint64_t>::iterator it = D.begin(), eit = D.end(); it != eit; ++it) {
		i++;
		if (i == SAMPLING_RATE) {
			i = 1;
			tmp.insert(*it);
		}
	}

	D.clear();
	D = tmp;

	return;
}

#define THRESHOLD 8
#define BLOCK_NUM 256

inline uint64_t valueToBin(uint64_t value) {
	if (value < ((uint64_t)1 << THRESHOLD)) {
		return value;
	} else {
		uint64_t base;
		for (int i = THRESHOLD+1; i < 64; i++) {
			if (value < ((uint64_t)1 << i)) {
				base = ((uint64_t)1 << (i-1));
				return base + (value - base) / (base / BLOCK_NUM) * (base / BLOCK_NUM);
			}
		}
		base = ((uint64_t)1 << 63);
		return base + (value - base) / (base / BLOCK_NUM) * (base / BLOCK_NUM);
	}
}

void intraRT_cal(uint64_t addr, uint64_t time) {

	if (prevs.find(addr) != prevs.end()) {
		if (intraRT.find(time - prevs[addr]) != intraRT.end()) {
			intraRT[time - prevs[addr]] += 1;
		} else {
			intraRT[time - prevs[addr]] = 1;
		}
	}

	return;
}

void access(uint64_t addr, uint64_t wgid) {

	ref_time++;		
	trace_len++;

	addr = addr * DTS / CLS;

	if (f.find(wgid) == f.end()) {
		if (prevs.empty() == false) {
			for (std::unordered_map<uint64_t, uint64_t>::iterator it = prevs.begin(), eit = prevs.end(); it != eit; ++it) {
				e[wgid-1][it->first] = ref_time - it->second;
			}
			L[wgid-1] = ref_time - 1;
			prevs.clear();
		}
		ref_time = 1;
		f[wgid][addr] = ref_time;
		
		intraRT_cal(addr, ref_time);
		prevs[addr] = ref_time;
		
		if (N < wgid+1) {
			N = wgid+1;
		}

	} else {

		intraRT_cal(addr, ref_time);

		if (f[wgid].find(addr) == f[wgid].end()) {
			f[wgid][addr] = ref_time;
		} else {
			rt[wgid][addr] = ref_time - prevs[addr];
		}
		prevs[addr] = ref_time;
	}
	
	D.insert(addr);

	return;
}

void reset() {
	f.clear();
	e.clear();
	rt.clear();
	prevs.clear();
	
	N = 0;
	ref_time = 0;
	interRT.clear();		
	L.clear();
	D.clear();
	
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

double averagedFCount(int n, int m, int i) {

	double res = 1;
	for (int j = 0; j < i; j++) {
		res *= double (m-j) / (n-j);
	}
	return res / (n-i);
}

void fRT_cal(uint64_t addr, uint64_t avgL, uint64_t m) {

	for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
		if (fit->second.find(addr) != fit->second.end()) {
			for (int i = 0; i <= m; i++) {
				uint64_t tmpRT = fit->second[addr] + i * avgL;
				if (fRT.find(tmpRT) != fRT.end()) {
					fRT[tmpRT] += averagedFCount(N, m, i);
				} else {
					fRT[tmpRT] = averagedFCount(N, m, i);
				}
			}
		}
	}	

	return;
}

double averagedECount(int n, int m, int i) {
	
	double res = 1;
	for (int j = 0; j < i; j++) {
		res *= double (m-j) / (n-j);
	}
	return res / (n-i);
}

void eRT_cal(uint64_t addr, uint64_t avgL, uint64_t m) {
	
	for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
		if (eit->second.find(addr) != eit->second.end()) {
			for (int i = 0; i <= m; i++) {
				uint64_t tmpRT = eit->second[addr] + i * avgL;
				if (eRT.find(tmpRT) != eRT.end()) {
					eRT[tmpRT] += averagedECount(N, m, i);
				} else {
					eRT[tmpRT] = averagedECount(N, m, i);
				}
			}
		}
	}	

	return;
}

void init_sol() {

	//cout << "Start to finilize" << endl;
	for (unordered_map<uint64_t, uint64_t>::iterator it = prevs.begin(), eit = prevs.end(); it != eit; ++it) {
		e[N-1][it->first] = ref_time - it->second + 1;
	}
	L[N-1] = ref_time;
	//cout << "End finilizing" << endl;

	vector<uint64_t> order(N);
	for (uint64_t i = 0; i < N; i++) {
		order[i] = i;
		//cout << order[i] << " ";
	}

	/* generating random order */
	//std::default_random_engine engine(std::random_device{}());
	//shuffle(order.begin(), order.end(), engine);
	
	/* calculating the length between all work groups */
	unordered_map<uint64_t, unordered_map<uint64_t, uint64_t> > lenAll;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (j == 0) {
				lenAll[i][i] = L[order[i]]; 
			}
			if (i+j < N) {
				lenAll[i][i+j] = lenAll[i][i+j-1] + L[order[i+j]];
			}
		}
	}

	/* calculating inter work group reuse */
	unordered_map<uint64_t, uint64_t> wid_prev;
	for (uint64_t i = 0; i < N; i++) {
		uint64_t wid = order[i];
		cout << wid << " ";
	
		if (i == 0) {
			for (auto it = e[wid].begin(), eit = e[wid].end(); it != eit; ++it) {
				wid_prev[it->first] = i;
			}
		} else {
			for (auto it = f[wid].begin(), eit = f[wid].end(); it != eit; ++it) {
				if (wid_prev.find(it->first) != wid_prev.end()) {
					uint64_t wid_start = wid_prev[it->first];
					uint64_t rtTmp;
					if (wid_start + 1 <= i - 1) {
						rtTmp = e[wid_start][it->first] + lenAll[wid_start+1][i-1] + f[i][it->first];
					} else {
						rtTmp = e[wid_start][it->first] + f[i][it->first];
					}
					if (interRT.find(rtTmp) != interRT.end()) {
						interRT[rtTmp] += 1;
					} else {
						interRT[rtTmp] = 1;
					}
				}
			}
		}
	}

	/* calculating global first access time */
	wid_prev.clear();
	uint64_t tmpLen = 0;
	for (uint64_t i = 0; i < N; i++) {
		uint64_t wid = order[i];
		for (auto it = f[wid].begin(), eit = f[wid].end(); it != eit; ++it) {
			if (wid_prev.find(it->first) == wid_prev.end()) {
				uint64_t rtTmp = tmpLen + it->second;
				if (interRT.find(rtTmp) != interRT.end()) {
					interRT[rtTmp] += 1;
				} else {
					interRT[rtTmp] = 1;
				}
				wid_prev[it->first];
			}
		}
		tmpLen += L[wid];
	}

	/* calculating global exit time */
	wid_prev.clear();
	tmpLen = 0;
	for (uint64_t i = 0; i < N; i++) {
		uint64_t wid = order[N - 1 - i];
		for (auto it = e[wid].begin(), eit = e[wid].end(); it != eit; ++it) {
			if (wid_prev.find(it->first) == wid_prev.end()) {
				uint64_t rtTmp = tmpLen + it->second;
				if (interRT.find(rtTmp) != interRT.end()) {
					interRT[rtTmp] += 1;
				} else {
					interRT[rtTmp] = 1;
				}
				wid_prev[it->first];
			}
		}
		tmpLen += L[wid];
	}

	f.clear();
	e.clear();

	return;
}


void init_aeol() {
	/* finilize exit time calculation for the last work group */
	cout << "Number of work groups " << N << endl;

	cout << "Start to finilize" << endl;
	for (std::unordered_map<uint64_t, uint64_t>::iterator it = prevs.begin(), eit = prevs.end(); it != eit; ++it) {
		e[N-1][it->first] = ref_time - it->second + 1;
	}
	L[N-1] = ref_time;
	cout << "End finilizing" << endl;

#ifdef ADDRESS_SAMPLING
	/* Address Sampling */
	address_sampling();
#endif
	
#ifdef CLASSIFICATION

	cout << "Start to classify" << endl;
	for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {
		
		//cout << *it << endl;

		std::string tmp = "";
		for (int i = 0; i < N; i++) {
			if (f[i].find(*it)  == f[i].end()) {
				tmp.push_back('0');
			} else {
				tmp.push_back('1');
			}
		}
		
		if (CID.find(tmp) == CID.end()) {
			CID[tmp] = cid;
			CM[cid] = N;
			for (int i = 0; i < N; i++) {
				if (tmp[i] == '1') {
					CM[cid]--;
				}
			}
			CaveL[cid] = 0;
			for (int i = 0; i < N; i++) {
				if (tmp[i] == '0') {
					CaveL[cid] += L[i];
				}
			}
			if (CM[cid] != 0) {
				CaveL[cid] = double(CaveL[cid]) / CM[cid];
			}
			cid++;
		}	
	
		uint64_t id = CID[tmp];	
		//cout << id << endl;
		for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
			for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
				if (fit->first == eit->first) {
					continue;
				}
				if (fit->second.find(*it) == fit->second.end()) {
					continue;
				}
				if (eit->second.find(*it) == eit->second.end()) {
					continue;
				}
				//uint64_t rtTmp = valueToBin(fit->second[*it] + eit->second[*it]);
				uint64_t rtTmp = fit->second[*it] + eit->second[*it];
				if (CEF[id].find(rtTmp) != CEF[id].end()) {
					CEF[id][rtTmp] ++;
				} else {
					//cout << rtTmp << endl;
					CEF[id][rtTmp] = 1;
				}
			}
		}

		fRT_cal(*it, CaveL[id], CM[id]);
		eRT_cal(*it, CaveL[id], CM[id]);
	}
	cout << "End classify" << endl;

#else
	/* init D, F, E, M*/
	for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {

		M[*it] = 0;
		aveL[*it] = 0;

		//cout << "d " << *it <<  " N " << N << endl;
		for (int i = 0; i < N; i++) {
			if (f[i].find(*it) == f[i].end()) {
				aveL[*it] += L[i];
				M[*it]++;
			}
		}

		if (M[*it] != 0) {
			aveL[*it] = double(aveL[*it]) / M[*it];
		}

		//cout << "start F" << endl;

		F[*it] = new std::vector<uint64_t>();
		for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
			if (fit->second.find(*it) != fit->second.end()) {
				F[*it]->push_back(fit->second[*it]);
			}
		}

		//cout << "start E" << endl;

		E[*it] = new std::vector<uint64_t>();
		for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
			//cout << eit->first << " " << endl;
			if (eit->second.find(*it) != eit->second.end()) {
				E[*it]->push_back(eit->second[*it]);
			}
		}
	}
#endif

	return;
}

void alg2() {
	/* alg 2 */
	for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {
		//cout << "d " << *it << endl;
		for (std::vector<uint64_t>::iterator fit = F[*it]->begin(), efit = F[*it]->end(); fit != efit; ++fit) {
			//cout << "f " << *fit << " ";
			for (std::vector<uint64_t>::iterator eit = E[*it]->begin(), eeit = E[*it]->end(); eit != eeit; ++eit) {
				//cout << "e " << *eit << " ";
				for (int i = 0; i <= M[*it]; i++) {
					//cout << "m " << i << " avel " << aveL[*it] << " ";
					uint64_t tmpRT = *eit + aveL[*it] * i + *fit - 1;
					//cout << "rt " << tmpRT << endl;
					if (interRT.find(tmpRT) != interRT.end()) {
						interRT[tmpRT] += averagedCount(N, M[*it], i);
					} else {
						interRT[tmpRT] = averagedCount(N, M[*it], i);
					}
				}
			}
		}
		for (int j = 0; j < N; j++) {
			if (f[j].find(*it) != f[j].end()) {
				for (int i = 0; i <= M[*it]; i++) {
					uint64_t tmpRT = e[j][*it] + aveL[*it] * i + f[j][*it] - 1;
					interRT[tmpRT] -= averagedCount(N, M[*it], i);
				}
			}
		}
	}
	return;	
}

void alg3() {
	/* alg 3 */

	//for (std::set<std::bitset<N>>::iterator it = C.begin(), endit = C.end(); it != endit; ++it) {
	for (uint64_t c = 0; c < cid; c++) {
		for (std::unordered_map<uint64_t, uint64_t>::iterator cefit = CEF[c].begin(), ecefit = CEF[c].end(); cefit != ecefit; ++cefit) {
			for (int i = 0; i <= CM[c]; i++) {
				uint64_t tmpRT = cefit->first + CaveL[c] * i - 1;
				if (interRT.find(tmpRT) != interRT.end()) {
					interRT[tmpRT] += averagedCount(N, CM[c], i) * cefit->second;
				} else {
					interRT[tmpRT] = averagedCount(N, CM[c], i)  * cefit->second;
				}
			}
		}
	}
/*	
	for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {
		for (int j = 0; j < N; j++) {
			if (f[j].find(*it) != f[j].end()) {
				for (int i = 0; i <= M[*it]; i++) {
					uint64_t tmpRT = e[j][*it] + aveL[*it] * i + f[j][*it] - 1;
					interRT[tmpRT] -= averagedCount(N, M[*it], i);
				}
			}
		}		
	}
*/
	return;
}

void mergeRT() {
	//cout << "start to Merge RT" << endl;
	for (std::unordered_map<uint64_t, double>::iterator it = fRT.begin(), eit = fRT.end(); it != eit; ++it) {
		if (RT.find(it->first) != RT.end()) {
			RT[it->first] += it->second;
		} else {
			RT[it->first] = it->second;
		}
	}

	for (std::unordered_map<uint64_t, double>::iterator it = eRT.begin(), eit = eRT.end(); it != eit; ++it) {
		if (RT.find(it->first) != RT.end()) {
			RT[it->first] += it->second;
		} else {
			RT[it->first] = it->second;
		}
	}

	for (std::unordered_map<uint64_t, uint64_t>::iterator it = intraRT.begin(), eit = intraRT.end(); it != eit; ++it) {
		if (RT.find(it->first) != RT.end()) {
			RT[it->first] += it->second;
		} else {
			RT[it->first] = it->second;
		}
	}

	for (std::unordered_map<uint64_t, double>::iterator it = interRT.begin(), eit = interRT.end(); it != eit; ++it) {
		if (RT.find(it->first) != RT.end()) {
			RT[it->first] += it->second;
		} else {
			RT[it->first] = it->second;
		}
	}
	//cout << "end merge" << endl;

	return;

}

void RTtoFP() {

	//cout << "Cal fp" << endl;
	uint64_t m = D.size();
	//cout << "m " << m << endl;
	//cout << "trace len " << trace_len << endl;
	//for (int i = 1; i <= trace_len; i++) {
	int i = 1;
	while (i < trace_len) {	
		
		double tmp = m * (trace_len - i + 1);
		for (std::map<uint64_t, double>::iterator it = RT.begin(), eit = RT.end(); it != eit; ++it) {
			if (it->first > i) {
				tmp -= it->second * (it->first - i);
			}
		}
		FP[i] = tmp / (trace_len - i + 1); 
	
		if (i < ((uint64_t)1 << THRESHOLD)) {
			i++;
		} else {
			i += i / BLOCK_NUM;
		}
	}

	return;
}


void sol() {
	init_sol();

	mergeRT();

	//RTtoFP();

	RTtoMR_AET();

	return;
}


void aeol() {

	init_aeol();

#ifdef CLASSIFICATION
	cout << "Start alg3" << endl;
	alg3();
#else
	cout << "Start alg2" << endl;
	alg2();
#endif	

	mergeRT();
	
	//RTtoFP();

	RTtoMR_AET();

	return;
}

void dumpfRT() {
	for (std::unordered_map<uint64_t, double>::iterator it = fRT.begin(), eit = fRT.end(); it != eit; ++it) {
		cout << it->first << " " << it->second << endl;
	}
	return;
}

void dumpeRT() {
	for (std::unordered_map<uint64_t, double>::iterator it = eRT.begin(), eit = eRT.end(); it != eit; ++it) {
		cout << it->first << " " << it->second << endl;
	}
	return;
}

void dumpIntraRT() {

	for (std::unordered_map<uint64_t, uint64_t>::iterator it = intraRT.begin(), eit = intraRT.end(); it != eit; ++it) {
		cout << it->first << " " << it->second << endl;
	}
	return;
}

void dumpInterRT() {
	for (std::unordered_map<uint64_t, double>::iterator it = interRT.begin(), eit = interRT.end(); it != eit; ++it) {
		if (it->second != 0) {
			cout << it->first << "	" << it->second << endl;
		}
	}
	return;
}

void dumpAllRT() {
	
	cout << "dump inter RT" << endl;
	dumpInterRT();
	cout << "dump intra RT" << endl;
	dumpIntraRT();
	cout << "dump fRT" << endl;
	dumpfRT();
	cout << "dump eRT" << endl;
	dumpeRT();

	return;
}

void dumpD() {
	for (std::set<uint64_t>::iterator it = D.begin(), eit = D.end(); it != eit; ++it) {
		cout << *it << " ";
	}
	cout << endl;
	return;
}

void dumpf() {
	for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
		cout << "work group " << fit->first << endl;
		for (std::unordered_map<uint64_t, uint64_t>::iterator jt = fit->second.begin(), ejt = fit->second.end(); jt != ejt; ++jt) {
			cout << "  d " << jt->first << " f " << jt->second << endl;
		}
	}
	return;
}

void dumpe() {
	for (std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
		cout << "work group " << eit->first << endl;
		for (std::unordered_map<uint64_t, uint64_t>::iterator jt = eit->second.begin(), ejt = eit->second.end(); jt != ejt; ++jt) {
			cout << "  d " << jt->first << " e " << jt->second << endl;
		}
	}
	return;
}

void dumpL() {
	for (std::unordered_map<uint64_t, uint64_t>::iterator it = L.begin(), eit = L.end(); it != eit; ++it) {
		cout << " work group " << it->first << " l " << it->second << endl; 
	}		
	return;
}

void dumpC() {
	cout << "Total number of classes " << cid << endl;
	for (std::unordered_map<std::string, uint64_t>::iterator it = CID.begin(), eit = CID.end(); it != eit; ++it) {
		cout << it-> first << endl;
	}
	return;
}	

