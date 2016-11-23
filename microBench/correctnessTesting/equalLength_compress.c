#include "../../utilities/aeol_compress.h"

void sequence1() {
	int n = 100;
	int m = 30;
	for (int i = 0; i < n; i++) {
		for (int d = 0; d < m; d++) {
			accessAEOLC(d, i);
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
				accessAEOLC(d, i);
			} else {
				accessAEOLC(m-1-d, i);
			}
		}
	}
	return;
}

int main() {

	printf("Repeating n \"1,2,...,m\" sequence\n");
	sequence1();
	aeolc();
	dumpInterRTC();
	reset();

	printf("Repeating n/2 \"1,2,...,m m,...,2,1\" sequence\n");
	sequence2();
	aeolc();
	dumpInterRTC();
	reset();

	return 0;	
}
