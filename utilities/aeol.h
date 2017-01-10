#include <stdlib.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <bitset>
using namespace std;

/* raw data */
std::map<uint64_t, std::map<uint64_t, uint64_t> > f;
std::map<uint64_t, std::map<uint64_t, uint64_t> > e;
std::map<uint64_t, std::map<uint64_t, uint64_t> > rt;
std::map<uint64_t, uint64_t> L;
std::map<uint64_t, uint64_t> prevs;
std::set<uint64_t> D;
std::map<uint64_t, double> interRT;

uint64_t ref_time = 0;

uint64_t N = 0;

/* processed data for alg 2 */
std::map<uint64_t, std::vector<uint64_t>* > F;
std::map<uint64_t, std::vector<uint64_t>* > E;
std::map<uint64_t, uint64_t> M;
std::map<uint64_t, uint64_t> aveL;

/* processed data for alg 3 */
uint64_t cid = 0;
std::map<std::string, uint64_t> CID;
std::map<uint64_t, std::map<uint64_t, uint64_t> > CEF;
std::map<uint64_t, uint64_t> CM;
std::map<uint64_t, uint64_t> CaveL;


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


void access(uint64_t addr, uint64_t wgid) {

	ref_time++;		

	if (f.find(wgid) == f.end()) {
		if (prevs.empty() == false) {
			for (std::map<uint64_t, uint64_t>::iterator it = prevs.begin(), eit = prevs.end(); it != eit; ++it) {
				e[wgid-1][it->first] = ref_time - it->second;
			}
			L[wgid-1] = ref_time - 1;
			prevs.clear();
		}
		ref_time = 1;
		f[wgid][addr] = ref_time;
		prevs[addr] = ref_time;
		
		if (N < wgid+1) {
			N = wgid+1;
		}

	} else {
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


void init_aeol() {
	/* finilize exit time calculation for the last work group */
	cout << "Start to finilize" << endl;
	for (std::map<uint64_t, uint64_t>::iterator it = prevs.begin(), eit = prevs.end(); it != eit; ++it) {
		e[N-1][it->first] = ref_time - it->second + 1;
	}
	L[N-1] = ref_time;

#ifdef ADDRESS_SAMPLING
	/* Address Sampling */
	address_sampling();
#endif
	
#ifdef CLASSIFICATION


	for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {
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
		for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
			for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
				if (fit->first == eit->first) {
					continue;
				}
				if (fit->second.find(*it) == fit->second.end()) {
					continue;
				}
				if (eit->second.find(*it) == eit->second.end()) {
					continue;
				}
				uint64_t rtTmp = fit->second[*it] + eit->second[*it];
				if (CEF[id].find(rtTmp) != CEF[id].end()) {
					CEF[id][rtTmp] ++;
				} else {
					//cout << rtTmp << endl;
					CEF[id][rtTmp] = 1;
				}
			}
		}
	}

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
		for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
			if (fit->second.find(*it) != fit->second.end()) {
				F[*it]->push_back(fit->second[*it]);
			}
		}

		//cout << "start E" << endl;

		E[*it] = new std::vector<uint64_t>();
		for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
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
		for (std::map<uint64_t, uint64_t>::iterator cefit = CEF[c].begin(), ecefit = CEF[c].end(); cefit != ecefit; ++cefit) {
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

void aeol() {

	init_aeol();

#ifdef CLASSIFICATION
	alg3();
#else
	alg2();
#endif	

	return;
}

void dumpInterRT() {
	for (std::map<uint64_t, double>::iterator it = interRT.begin(), eit = interRT.end(); it != eit; ++it) {
		if (it->second != 0) {
			cout << it->first << "	" << it->second << endl;
		}
	}
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
	for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator fit = f.begin(), efit = f.end(); fit != efit; ++ fit) {
		cout << "work group " << fit->first << endl;
		for (std::map<uint64_t, uint64_t>::iterator jt = fit->second.begin(), ejt = fit->second.end(); jt != ejt; ++jt) {
			cout << "  d " << jt->first << " f " << jt->second << endl;
		}
	}
	return;
}

void dumpe() {
	for (std::map<uint64_t, std::map<uint64_t, uint64_t> >::iterator eit = e.begin(), eeit = e.end(); eit != eeit; ++ eit) {
		cout << "work group " << eit->first << endl;
		for (std::map<uint64_t, uint64_t>::iterator jt = eit->second.begin(), ejt = eit->second.end(); jt != ejt; ++jt) {
			cout << "  d " << jt->first << " e " << jt->second << endl;
		}
	}
	return;
}

void dumpL() {
	for (std::map<uint64_t, uint64_t>::iterator it = L.begin(), eit = L.end(); it != eit; ++it) {
		cout << " work group " << it->first << " l " << it->second << endl; 
	}		
	return;
}

void dumpC() {
	cout << "Total number of classes " << cid << endl;
	for (std::map<std::string, uint64_t>::iterator it = CID.begin(), eit = CID.end(); it != eit; ++it) {
		cout << it-> first << endl;
	}
	return;
}	

