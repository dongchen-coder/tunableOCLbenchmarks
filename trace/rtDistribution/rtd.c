#include "../intraEOinsensitiveTrace/gemm.h"
#include "../../utilities/rtd.h"

int main() {

	void (*f1) (uint64_t, uint64_t) = accessRTD;
	void (*f2) () = resetRTD;
	void (*f3) () = calculateRTD;

	printf("Profiling reuse distribution for GEMM\n");
	gemm_main(f1, f2, f3);
	
	return 0;
}
