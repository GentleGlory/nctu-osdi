#include "user_lib.h"


void main(void) { // test page reclaim.
	printf("Remaining page frames : %llu\n", remain_page_num()); // get number of remaining page frames from kernel by system call.

	exit(0);
}