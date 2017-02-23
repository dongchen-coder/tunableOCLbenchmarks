#include "../intraEOinsensitiveTrace/3dConvolution.h"

#include "../../utilities/aeol.h"


int main(int argc, char* argv[]) {

	void (*f1) (uint64_t, uint64_t) = access;
	void (*f2) () = reset;
	void (*f3) () = sol;
	//void (*f4) () = dumpALL;
	void (*f4) () = dumpMR;

	int cX, cY;

	if (argc != 3) {
		cout << "Not enough arguments" << endl;
	} else {
		cX = stoi(argv[1]);
		cY = stoi(argv[2]);
	}

	convolution3d_main(f1, f2, f3, f4, cX, cY);

	reset();

	return 0;
}


