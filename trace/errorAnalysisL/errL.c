#include "../intraEOinsensitiveTrace/gemm.h"
#include "../../utilities/errorAnalysisL.h"

void test() {
	accessErrL(1, 0);
	accessErrL(2, 0);
	accessErrL(3, 0);
	accessErrL(1, 1);
    accessErrL(2, 1);
    //accessErrL(2, 1);
	accessErrL(1, 2);
    accessErrL(2, 2);
    accessErrL(3, 2);
	accessErrL(1, 3);
    accessErrL(2, 3);
    //accessErrL(2, 3);
	calculateErrL();
	return;
}


int main() {
	/*
	test();
	*/
	
	void (*f1) (uint64_t, uint64_t) = accessErrL;
	void (*f2) () = resetErrL;
	void (*f3) () = calculateErrL;

	printf("Profiling reuse distribution for GEMM\n");
	gemm_main(f1, f2, f3);
	
	return 0;
}
