#include "exception.h"
#include "string.h"
#include "irq.h"
#include "system_call.h"
#include "core.h"

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

void el1_sync_exc_router(struct pt_regs* ptregs)
{
	uint64_t esr = read_sys_reg(esr_el1);
	int ec = EXC_ESR_EC(esr);

	printf("\r el1 sync_exc_router, esr:%llx, elr:%llx\n",
			esr, ptregs->elr);
	printf("\r Exception class (EC): 0x%x, ISS: 0x%x\n",
			ec, (int)(esr & 0xFFFFFF));

	// For now, hang for debugging data abort and other exceptions
	// In a production system, you would handle different exception types
	while(1);
}

void el0_sync_exc_router(struct pt_regs* ptregs)
{
	uint64_t esr = read_sys_reg(esr_el1);

	int ec = EXC_ESR_EC(esr);
	
	if (ec == EXC_ESR_EC_SVC_ARM64) {
		system_call_exc_handler(ptregs);		
	} else {
		printf("\r el0 sync_exc_router, esr:%llx, elr:%llx\n",
			esr, ptregs->elr);
			
		// Hang the system for debugging
		while(1);
	}
}

void irq_exc_router()
{
	irq_handler();
}