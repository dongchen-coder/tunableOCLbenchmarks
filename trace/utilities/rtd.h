
#include <stdlib.h>
#include <map>
#include <set>
using namespace std;

std::set<uint64_t> D;
std::map<uint64_t, std::map<uint64_t, uint64_t> > DtoC;

void accessRTD(uint64_t addr, uint64_t wgid) {
	
	if (DtoC.find(wgid) != DtoC.end()) {
		if (DtoC[wgid].find(addr) != DtoC[wgid].end()) {
			DtoC[wgid][addr] += 1;
		} else {
			DtoC[wgid][addr] = 1;
		}
	} else {
		DtoC[wgid][addr] = 1;
	}

	D.insert(addr);
	return;
}

void calculateRTD() {
    uint64_t intraC = 0;
    uint64_t interC = 0;
    double intraD = 0;
    double interD = 0;
	
	int N = DtoC.size();

    for (int i = 0; i < N; i++) {
        for (std::map<uint64_t, uint64_t>::iterator it = DtoC[i].begin(), eit = DtoC[i].end(); it != eit; ++it) {
            intraC += it->second;
        }
    }

    for (std::set<uint64_t>::iterator it = D.begin(), endit = D.end(); it != endit; ++it) {
        int tmp = 0;
        for (int i = 0; i < N; i++) {
            if (DtoC[i].find(*it) != DtoC[i].end()) {
                tmp++;
            }
        }
        interC += tmp - 1;
    }

    intraD = (double) intraC / (intraC + interC);
    interD = (double) interC / (intraC + interC);

    printf("Intra vs Inter reuse count %llu, %llu\n", intraC, interC);
    printf("              distribution %f, %f\n", intraD, interD);

    return;
}

void resetRTD() {
	D.clear();
	DtoC.clear();
	return;
}


