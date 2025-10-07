#include "exception.h"
#include "string.h"
#include "irq.h"
#include "system_call.h"

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

void sync_exc_router(uint64_t esr, uint64_t elr, uint64_t syscall_num)
{
	printf("\rIn sync_exc_router, esr:0x%llx, elr:0x%llx\n",
			esr, elr);

	int ec = EXC_ESR_EC(esr);

	if (ec == EXC_ESR_EC_SVC_ARM64) {
		system_call_handler(syscall_num, esr, elr);
	} else {
		printf("\rUnhandeled ec:%x\n", ec);
	}
}

void irq_exc_router()
{
	irq_handler();
}