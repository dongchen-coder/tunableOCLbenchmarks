#include "../intraEOinsensitiveTrace/3dConvolution.h"

#include "../../utilities/aeol.h"


int main() {

	void (*f1) (uint64_t, uint64_t) = access;
    void (*f2) () = reset;
    void (*f3) () = aeol;
	void (*f4) () = dumpMR;

	convolution3d_main(f1, f2, f3, f4);

	return 0;
}


