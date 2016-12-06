//#include "../intraEOinsensitiveTrace/gemm.h"
//#include "../intraEOinsensitiveTrace/3mm.h"
//#include "../intraEOinsensitiveTrace/atax.h"
//#include "../intraEOinsensitiveTrace/bicg.h"
//#include "../intraEOinsensitiveTrace/gemver.h"
//#include "../intraEOinsensitiveTrace/gesummv.h"
//#include "../intraEOinsensitiveTrace/mvt.h"
//#include "../intraEOinsensitiveTrace/syrk.h"
#include "../intraEOinsensitiveTrace/2dConvolution.h"
//#include "../intraEOinsensitiveTrace/3dConvolution.h"

#include "../../utilities/rtd.h"

int main() {

	void (*f1) (uint64_t, uint64_t) = accessRTD;
	void (*f2) () = resetRTD;
	void (*f3) () = calculateRTD;

	//printf("Profiling reuse distribution for GEMM\n");
	//gemm_main(f1, f2, f3);
	
	//printf("Profiling reuse distribution for 3mm\n");
	//threeMMmain(f1, f2, f3);

	//printf("Profiling reuse distribution for atax\n");
	//atax_main(f1, f2, f3);

	//printf("Profiling reuse distribution for bicg\n");
	//bicg_main(f1, f2, f3);

	//printf("Profiling reuse distribution for gemver\n");
	//gemver_main(f1, f2, f3);

	//printf("Profiling reuse distribution for gesummv\n");
	//gesummv_main(f1, f2, f3);

	//printf("Profiling reuse distribution for mvt\n");
	//mvt_main(f1, f2, f3);

	//printf("Profiling reuse distribution for syrk\n");
	//syrk_main(f1, f2, f3);

	printf("Profiling reuse distribution for 2dConvolution\n");
	convolution2d_main(f1, f2, f3);

	//printf("Profiling reuse distribution for 3dConvolution\n");
	//convolution3d_main(f1, f2, f3);

	return 0;
}
