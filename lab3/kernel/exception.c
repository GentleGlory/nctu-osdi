#include "exception.h"
#include "string.h"

void show_exception_status(uint64_t type, uint64_t esr, uint64_t elr)
{
	printf("\rType:%llu, ESR: 0x%llx, address: 0x%llx\n",
			type, esr, elr);
	printf("\rException class (EC) 0x%llx\n", (esr >> 26) & 0b111111);
	printf("\rInstruction specific syndrome (ISS) 0x%llx\n", esr & 0xFFFFFF);
}

void not_implemented()
{
	printf("\rkenel panic because of not implemented function...\n");
	while (1);
}

void sync_exc_router(uint64_t esr, uint64_t elr) 
{
	printf("\rIn sync_exc_router, esr:0x%llx, elr:0x%llx\n",
			esr, elr);

	int ec = (esr >> 26) & 0b111111;
	int iss = esr & 0x1FFFFFF;    

	switch(iss) {
	case 1:
		printf("\rException return address 0x%x\n", elr);
		printf("\rException class (EC) 0x%x\n", ec);
		printf("\rInstruction specific syndrome (ISS) 0x%x\n", iss);
	break;
	default:
		printf("\rUnhandeled iss:%x\n",iss);
	}
}