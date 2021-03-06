#include <stdlib.h>
#include <iostream>
#include <map> 
#include <fstream>
#include <bitset>
#include "stdint.h"
using namespace std;

#define THRESHOLD 8
#define BLOCK_NUM 256

bool firstOpen = true;

unsigned long ref_time = 0;
uint64_t totalWrites = 0;
map<uint64_t, uint64_t> wtmp;
map<uint64_t, uint64_t> wrt_histo;
map<uint64_t, uint64_t> prevs;
map<uint64_t, uint64_t> rt_cnt_histo;
map<uint64_t, uint64_t> rt_value_histo;
map<uint64_t, uint64_t> footprint;
map<uint64_t, double> footprint_avg;
map<uint64_t, uint64_t> wb;
map<uint64_t, uint64_t> w;
map<uint64_t, uint64_t> twb;

//map<uint64_t, uint64_t> first_access_time;

std::string fileNameShared;

double mr;
uint64_t cache_size;

#define CLS 32
#define DS 4

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

int writeRTHisto(uint64_t addr, unsigned long ref_time, char wr) {

	if (prevs.find(addr) != prevs.end()) {
		if (wr == 'W') {
			wtmp[addr] = 0;
		}
	}

	if (wr == 'W') {
		if (wtmp.find(addr) != wtmp.end()) {
			uint64_t maxRT;
			if(wtmp[addr] > ref_time - prevs[addr]) {
				maxRT = wtmp[addr];	
			} else {
				maxRT = ref_time - prevs[addr];
			}
			if (wrt_histo.find(maxRT) != wrt_histo.end()) {
				wrt_histo[maxRT] += 1;
				//wrt_histo[valueToBin(maxRT)] += 1;
			} else {
				wrt_histo[maxRT] = 1;
				//wrt_histo[valueToBin(maxRT)] += 1;
			}
			wtmp[addr] = 0;
		} else {
			wtmp[addr] = 0;	
		}	
	} else {
		if (wtmp.find(addr) != wtmp.end()) {
			if (wtmp[addr] < ref_time - prevs[addr]) {
				wtmp[addr] = ref_time - prevs[addr];
			}
		}   
	}
	 
	return 0;
}


int RTtoFP() {

	ref_time+=1;

	// accumulate last access to the reuse interval histogram
	for (std::map<uint64_t,uint64_t>::iterator it=prevs.begin(); it!=prevs.end(); ++it) {
		rt_cnt_histo[valueToBin(ref_time - it->second)] += 1;
		rt_value_histo[valueToBin(ref_time - it->second)] += ref_time - it->second; 
	}
	
	// excluding the first access times
	/* 
	for (std::map<uint64_t,uint64_t>::iterator it = first_access_time.begin(), eit = first_access_time.end(); it != eit; ++it) {
		rt_cnt_histo[valueToBin(it->second)] --;
		rt_value_histo[valueToBin(it->second)] -= it->second;
	}
	*/
	
	// calculate the window sizes in log scale after THRESHOLD
	for (uint64_t i = 1; i < (1 << THRESHOLD); i++) {	
		w[i] = i;
	}
	for (uint64_t i = THRESHOLD; i < 64; i++) {
		uint64_t base = (uint64_t)1 << i;
		for (uint64_t j = 0; j < BLOCK_NUM; j++) {
			w[base+j] = base + j * base / BLOCK_NUM;
		}
	} 

	// calculate footprint
	map<uint64_t, uint64_t> sum;
	for (std::map<uint64_t, uint64_t>::iterator it=w.begin(); it!= w.end(); ++it) {
		uint64_t wi = it->second;
		for (std::map<uint64_t, uint64_t>::iterator jt=it; jt!= w.end(); ++jt) {
			uint64_t wj = jt->second;
			sum[wi] += rt_value_histo[wj] - rt_cnt_histo[wj] *  wi;
		} 
	}
	for (std::map<uint64_t, uint64_t>::iterator it=w.begin(); it!= w.end(); ++it) {
		uint64_t wi = it->second;
		if (ref_time > wi)
			footprint_avg[wi] = prevs.size() - sum[wi] * 1.0 / (ref_time - wi);
	}
	
	return 0;
}

/*
int RTHistoFirstAccessTimeFilter(uint64_t addr, unsigned long ref_time) {
	if (first_access_time.find(addr) == first_access_time.end()) {
		first_access_time[addr] = ref_time;
	}
	return 0;
}*/


int RTHisto(uint64_t addr, unsigned long ref_time) {

	if (prevs.find(addr) != prevs.end()) {
		uint64_t rt = ref_time - prevs[addr];
				rt = valueToBin(rt);
		if (rt_cnt_histo.find(rt) == rt_cnt_histo.end()) {
		rt_cnt_histo[rt] = 1;
		  rt_value_histo[rt] = ref_time - prevs[addr];
		} else {
	  	rt_cnt_histo[rt] += 1;
			rt_value_histo[rt] += ref_time - prevs[addr];
	 	}
	} else {
		if (rt_cnt_histo.find(valueToBin(ref_time)) != rt_cnt_histo.end()) {
		  rt_cnt_histo[valueToBin(ref_time)] += 1;
					rt_value_histo[valueToBin(ref_time)] += ref_time;
		} else {
		  rt_cnt_histo[valueToBin(ref_time)] = 1;
					rt_value_histo[valueToBin(ref_time)] = ref_time;
		}
	}
	prevs[addr] = ref_time;
	return 0;
}

// calculate miss ratio from footprint for specific cache size
double FPtoMR(int cacheSize) {

	/*
	uint64_t n = 0;
	uint64_t m = 0;
	
	for (std::map<uint64_t, uint64_t>::iterator it = rt_cnt_histo.begin(); it != rt_cnt_histo.end(); ++it){
		n += it->second;
		if (footprint_avg[it->first] >= cacheSize) {
			m += it->second;
		}	
	}
	return ((double) m) / n;
	*/
	
	uint64_t prev_win = 0;
	double prev_fp = 0;
	for (std::map<uint64_t, double>::iterator it=footprint_avg.begin(); it!=footprint_avg.end(); ++it) {
		if (it->first > cache_size) {
			return double(it->second - prev_fp) / (it->first - prev_win);
		}
		prev_win = it->first;
		prev_fp = it->second;
	}

	return 0;
}

// calculate write backs
int WBHisto(void) {

	cout << "Start to calculate write reuse intervals for last writes before flush" << endl;
	for (std::map<uint64_t,uint64_t>::iterator it=wtmp.begin(); it!=wtmp.end(); ++it) {
		uint64_t maxRT;
		if (it->second > ref_time - prevs[it->first]) {
			maxRT = it->second;
		} else {
			maxRT = ref_time - prevs[it->first];
		}
		wtmp[it->first] = maxRT;
		if (wrt_histo.find(maxRT) != wrt_histo.end()) {
			wrt_histo[maxRT] += 1;
		} else {
			wrt_histo[maxRT] = 1;
		}
	}

	cout << "Finish calculating write reuse intervals" << endl;
	for (std::map<uint64_t, uint64_t>::iterator it = w.begin(); it != w.end(); ++it) {
		uint64_t i = it->second;
		uint64_t filltime = 0;
		uint64_t j;

		std::map<uint64_t, double>::iterator jt = footprint_avg.begin();
		std::map<uint64_t, double>::iterator jt_pre = jt;
		++jt;
		for (; jt != footprint_avg.end(); ++jt) {
			if (i >= jt_pre->second && i < jt->second) {
				filltime = jt_pre->first + (jt->first - jt_pre->first) * (i - jt_pre->second) / (jt->second - jt_pre->second);
				break;
			}
			++jt_pre;
		}

		if (filltime == 0 && jt == footprint_avg.end()) {
			filltime = ref_time;
		}
		std::map<uint64_t,uint64_t>::iterator kt; 
		if (wrt_histo.find(filltime) != wrt_histo.end()) {
			j = filltime;
		} else {
			for (kt=wrt_histo.begin(); kt != wrt_histo.end(); ++kt) {
				if (kt->first > filltime) {
					j = kt->first;
					break;
				}
			}
			if (kt == wrt_histo.end()) {
				break;
			}
		}	
	 
		for (kt = wrt_histo.begin(); kt != wrt_histo.end(); ++kt) {
			if (kt->first == j) {
				break;
			}
		}
		
		for (; kt != wrt_histo.end(); ++kt) {
			wb[i] += kt->second;
		}

		// terminating wb
		twb[i] = 0;
		for (kt = wtmp.begin(); kt != wtmp.end(); ++kt) {
			if (kt->second <= filltime) {
				twb[i] += 1;	
			}
		}
	}

	return 0;
}


void RTtoMR() {
	
	cache_size = 49152;
	//cache_size = 32768;

	RTtoFP();
	mr = FPtoMR(cache_size);

	return;
}

void dumpMR() {

	cout << "Cache size " << double (cache_size * 32) / 1024 / 1024 << " MB" << endl;
	cout << "mr " << mr << endl;
	return;
}


void access(uint64_t addr, uint64_t wid) {
	ref_time++;
	addr = addr * DS / CLS;
	RTHisto(addr, ref_time);
	return;
}

void access(uint64_t addr, char wr) {
	
	ref_time++;
	if (wr == 'W') {
		totalWrites += 1;
		if (totalWrites == 0) {
			cout << "overflow" << endl;
		}
	}
	addr = addr * DS / CLS;
	
	writeRTHisto(addr, ref_time, wr);
	//RTHistoFirstAccessTimeFilter(addr, ref_time);
	
	RTHisto(addr, ref_time);
	return;
}

void dumpFP() {
	for (std::map<uint64_t, double>::iterator it=footprint_avg.begin(); it!=footprint_avg.end(); ++it) {
				cout << "win " << it->first << " footprint " << it->second << endl;
	}
	return;
}

int dump(char* fileName) {
	
	int fileNameLen = 0;
	while( fileName[fileNameLen] != '\0' && fileNameLen < 200) {
		if (fileName[fileNameLen] == '/' || fileName[fileNameLen] == '.') {
			fileName[fileNameLen] = '_';
		}
		fileNameLen++;
	}
	std::string fileName_s(fileName, fileNameLen);
	
	fileName_s = "/localdisk/dchen39/research/test/wbData/" + fileName_s + "Sim";
	
	fileNameShared = fileName_s;

	std::fstream fs;
	if (firstOpen == true) {
		fs.open (fileName_s.c_str(),  std::fstream::out | std::fstream::trunc);
		firstOpen = false;
	}
	else
		fs.open (fileName_s.c_str(),  std::fstream::out | std::fstream::app);

		for (std::map<uint64_t, double>::iterator it=footprint_avg.begin(); it!=footprint_avg.end(); ++it) {
				fs << "win " << it->first << " footprint " << it->second << endl;
		}

	for (std::map<uint64_t, uint64_t>::iterator it=wb.begin(); it!=wb.end(); ++it) {
		fs << "cache size " << it->first << " wb " << it->second << endl;
	}
	
	for (std::map<uint64_t, uint64_t>::iterator it=twb.begin(); it!=twb.end(); ++it) {
				fs << "cache size " << it->first << " twb " << it->second << endl;
		}

	fs.close();

	return 0;
}

void reset() {
	
	ref_time = 0;
	wtmp.clear();
	wrt_histo.clear();
	prevs.clear();
	rt_cnt_histo.clear();
	rt_value_histo.clear();
	footprint.clear();
	footprint_avg.clear();
	wb.clear();
	w.clear();
	twb.clear();

	//first_access_time.clear();

	return;
}


/*
int main() {

	for (uint64_t i = 0; i < 4; i++) {
		access(1, 'R');
		access(2, 'W');
		access(3, 'R');
		access(4, 'R');
	}

	cout << "finish access " << endl;
	
	RTtoFP();

	cout << "finish RT to FP" << endl;

	WBHisto();

	dump();

	return 0;
}
*/



