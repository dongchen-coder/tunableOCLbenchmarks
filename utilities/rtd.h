
#include <stdlib.h>
#include <map>
#include <set>
using namespace std;

std::set<uint64_t> D_w; 
std::map<uint64_t, uint64_t> DtoC;

uint64_t wgid_s = 0;
uint64_t intraC = 0;
uint64_t interC = 0;

void accessRTD(uint64_t addr, uint64_t wgid) {
	
	if (wgid != wgid_s) {
		wgid_s = wgid;
		for (std::set<uint64_t>::iterator it = D_w.begin(), eit = D_w.end(); it != eit; ++it) {
			if (DtoC.find(*it) != DtoC.end()) {
				DtoC[*it]++;
			} else {
				DtoC[*it] = 1;
			}
		}
		D_w.clear();
		D_w.insert(addr);
	} else {
		if(D_w.find(addr) != D_w.end()) {
			intraC++;	
		} else {
			D_w.insert(addr);
		}
	}

	return;
}

void calculateRTD() {
    double intraD = 0;
    double interD = 0;

	for (std::map<uint64_t, uint64_t>::iterator it = DtoC.begin(), eit = DtoC.end(); it != eit; ++it) {
		interC += it->second - 1;
	}

    intraD = (double) intraC / (intraC + interC);
    interD = (double) interC / (intraC + interC);

    printf("Intra vs Inter reuse count %llu, %llu\n", intraC, interC);
    printf("              distribution %f, %f\n", intraD, interD);

    return;
}

void resetRTD() {
	wgid_s = 0;
	intraC = 0;
	interC = 0;	
	D_w.clear();
	DtoC.clear();
	return;
}


