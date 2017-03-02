#include "../../intraEOinsensitiveTrace/atax.h"

#include "../../../utilities/aeol.h"


int main(int argc, char* argv[]) {

	void (*f1) (uint64_t, uint64_t) = access;
    void (*f2) () = reset;
    void (*f3) () = sol;
	void (*f4) () = dumpMR;

	int cX, cY, kID;
    if (argc != 4) {
        cout << "Not enough arguments" << endl;
    } else {
        cX = stoi(argv[1]);
        cY = stoi(argv[2]);
		kID = stoi(argv[3]);
    }

	atax_main(f1, f2, f3, f4, cX, cY, kID);

	return 0;
}


