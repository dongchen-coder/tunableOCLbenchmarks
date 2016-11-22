#include <stdio.h>
#include <map>
using namespace std;


std::map<uint64_t, uint64_t> D_w;

std::map<uint64_t, uint64_t> avgf;
std::map<uint64_t, uint64_t> avge;
std::map<uint64_t, uint64_t> avgl;
std::map<uint64_t, uint64_t> m;

std::map<uint64_t, double> interRTC;

uint64_t ref_time = 0;
uint64_t trace_len = 0;
uint64_t wgid_curr = 0;
uint64_t N = 0;

void accessAEOLC(uint64_t addr, uint64_t wgid) {
	
	ref_time++;
	trace_len++;

	if (wgid_curr != wgid) {
		for (std::map<uint64_t, uint64_t>::iterator it = D_w.begin(), eit = D_w.end(); it != eit; ++it) {
			if (avge.find(it->first) != avge.end()) {
				avge[it->first] += ref_time - it->second;
				avgl[it->first] += ref_time - 1;
			} else {
				avge[it->first] = ref_time - it->second;
				avgl[it->first] = ref_time - 1;
			}
		}
		N++;
		ref_time = 1;
		D_w.clear();
		D_w[addr] = ref_time;
		if (avgf.find(addr) != avgf.end()) {
                avgf[addr] += ref_time;
                m[addr] += 1;
            } else {
                avgf[addr] = ref_time;
                m[addr] = 1;
        }
		wgid_curr = wgid; 
	} else {
		if (D_w.find(addr) == D_w.end()) {
			if (avgf.find(addr) != avgf.end()) {
				avgf[addr] += ref_time;
				m[addr] += 1;
			} else {
				avgf[addr] = ref_time;
                m[addr] = 1;
			}
		}
		D_w[addr] = ref_time;
	}

	return;
}

double averagedCountCompressed(int n, int m, int i) {
    //printf("Caculate n m i %d %d %d\n", n, m, i);
    double res = 1;
    for (int j = 0; j < i; j++) {
        res *= double (m-j) / (n-j);
    }
    return res * (n - m) * (n - m) / (n-i);
}

void aeolc() {

	N++;	
	for (std::map<uint64_t, uint64_t>::iterator it = D_w.begin(), eit = D_w.end(); it != eit; ++it) {
		if (avge.find(it->first) != avge.end()) {
			avge[it->first] += ref_time - it->second;
			avgl[it->first] += ref_time - 1;
		} else {
			avge[it->first] = ref_time - it->second;
			avgl[it->first] = ref_time - 1;
		}
	}

	for (std::map<uint64_t, uint64_t>::iterator it = avgf.begin(), eit = avgf.end(); it != eit; ++it) {
		avgf[it->first] = it->second / m[it->first];
		avge[it->first] = avge[it->first] / m[it->first];
		avgl[it->first] = (trace_len - avgl[it->first]) / m[it->first];
		m[it->first] = N - m[it->first];
	}

	for (std::map<uint64_t, uint64_t>::iterator it = avgf.begin(), eit = avgf.end(); it != eit; ++it) {
		for (int i = 0; i <= m[it->first]; i++) {
			uint64_t tmpRT = it->second + avgl[it->first] * i + avge[it->first];
			if (interRTC.find(tmpRT) != interRTC.end()) {
				interRTC[tmpRT] += averagedCountCompressed(N, m[it->first], i);
			} else {
				interRTC[tmpRT] = averagedCountCompressed(N, m[it->first], i);
			}
		}
	}		

	return;
}

void reset() {

	D_w.clear();
	avgf.clear();
	avge.clear();
	avgl.clear();
	m.clear();
	interRTC.clear();

	ref_time = 0;
	trace_len = 0;
	wgid_curr = 0;
	N = 0;

	return;
}

void dumpInterRTC() {
	printf("dump compressed inter work group reuse\n");
	for (std::map<uint64_t, double>::iterator it = interRTC.begin(), eit = interRTC.end(); it != eit; ++it) {
		printf("%llu %f\n", it->first, it->second);
	}
	return;
}	
