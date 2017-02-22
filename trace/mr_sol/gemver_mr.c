#include "../intraEOinsensitiveTrace/gemver.h"

#include "../../utilities/aeol.h"


int main() {

	void (*f1) (uint64_t, uint64_t) = access;
    void (*f2) () = reset;
    void (*f3) () = sol;
	void (*f4) () = dumpALL;

	gemver_main(f1, f2, f3, f4);

	reset();

	return 0;
}


