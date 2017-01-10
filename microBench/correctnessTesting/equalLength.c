#include "../../utilities/aeol.h"

void sequence1() {
	int n = 100;
	int m = 30;
	for (int i = 0; i < n; i++) {
		for (int d = 0; d < m; d++) {
			access(d, i);
		}
	}
	return;
}

void sequence2() {
	int n = 100;
	int m = 30;
	for (int i = 0; i < n; i++) {
		for (int d = 0; d < m; d++) {
			if (i % 2 == 0) {
				access(d, i);
			} else {
				access(m-1-d, i);
			}
		}
	}
	return;
}

void stencil1D() {
	int n = 30;
	int m = 5;
	for (int i = 0; i < n; i++) {
		if (i == 0) {
			access(i, i/m);
			access(i+1, i/m);
		} else if (i == n-1) {
			access(i-1, i/m);
			access(i, i/m);
		} else {
			access(i-1, i/m);
			access(i, i/m);
			access(i+1, i/m);
		}
	}
}

void verify_stencil1D() {
	
	std::map<uint64_t, uint64_t> rt;
	std::map<uint64_t, uint64_t> pre;

	for (int i0 = 0; i0 < 6; i0++) {
		for (int i1 = 0; i1 < 6; i1++) {
			for (int i2 = 0; i2 < 6; i2++) {
				for (int i3 = 0; i3 < 6; i3++) {
					for (int i4 = 0; i4 < 6; i4++) {
						for (int i5 = 0; i5 < 6; i5++) {
							if ((i0 != i1) && (i0 != i2) && (i0 != i3) && (i0 != i4) && (i0 != i5)) {
							if ((i1 != i2) && (i1 != i3) && (i1 != i4) && (i1 != i5)) {
							if ((i2 != i3) && (i2 != i4) && (i2 != i5)) {
							if ((i3 != i4) && (i3 != i5)) {
							if (i4 != i5) {
								//printf("%d %d %d %d %d %d\n", i0, i1, i2, i3, i4, i5);						
								std::vector<uint64_t> order;
								order.push_back(i0);
								order.push_back(i1);
								order.push_back(i2);
								order.push_back(i3);
								order.push_back(i4);
								order.push_back(i5);
								
								uint64_t ref_time = 1;
								for(std::vector<uint64_t>::iterator it = order.begin(), eit = order.end(); it != eit; ++it) {
									for (int j = 0; j < 5; j++) {
										uint64_t access = (*it)*5 + j;
										
									}
								}
							}
							}
							}
							}
							}
						}
					}
				}
			}
		}
	}
	
	return;
}

int main() {
	/*
	printf("Repeating n \"1,2,...,m\" sequence\n");
	sequence1();
	aeol();
	dumpC();
	dumpInterRT();
	reset();
	*/
	printf("Repeating n/2 \"1,2,...,m m,...,2,1\" sequence\n");
	sequence2();
	aeol();
	dumpInterRT();
	reset();
	
	
	/*
	printf("1D stencil computation\n");
	stencil1D();
	aeol();
	dumpInterRT();
	reset();	

	verify_stencil1D();
	*/
	return 0;	
}
