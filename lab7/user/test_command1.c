#include "user_lib.h"

void main(void) { // test fork functionality
	int cnt = 0;
	if(fork() == 0) {
		fork();
		fork();
		while(cnt < 10) {
			printf("\rtask id: %d, sp: 0x%llx cnt: %d\n", get_task_id(), &cnt, cnt++); // address should be the same across tasks, but the cnt should be increased indepndently
			delay(1000);
		}
	}
}