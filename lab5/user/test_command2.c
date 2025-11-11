#include "user_lib.h"


void main(void) { // test page fault
	if(fork() == 0) {
		int* a = 0x0; // a non-mapped address.
		printf("\r%d\n", *a); // trigger simple page fault, child will die here.
	}

	exit(0);
}