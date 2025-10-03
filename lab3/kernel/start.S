#include "sys_regs.h"

#define EL2_SP  _start
#define EL1_SP  0x60000

.section ".text"
.align	3
_start:
	ldr x1, =EL2_SP
	mov sp,x1
	//Let only one core proceed, and let others enter a busy loop.
	mrs x0, mpidr_el1
	and x0, x0, #0xff
	cmp x0, 0
	b.eq 2f

1:	wfe
	b 1b

2:	//Clear BSS
	ldr x0, =__bss_start
	ldr x1, =__bss_end
	mov x2, #0
	mov x3, #0
3:	cmp x0, x1
	b.eq 4f
	stp x2, x3, [x0], #16
	b 3b

4:	mrs x0, hcr_el2
	ldr x1, =HCR_EL2_RW_AARCH64 
	orr x0, x0, x1
	msr hcr_el2, x0
	
	ldr x0, =SPSR_EL2_DAIF_DISABLE
	ldr x1, =SPSR_EL2_M_EL1H
	orr x0, x0, x1
	msr spsr_el2, x0

	//Enanle floating point and Advanced SIMD
	mrs x0, cpacr_el1
	ldr x1, =CPACR_EL1_FPEN
	orr x0, x0, x1
	msr cpacr_el1, x0

	adr x0, el1_start
	msr elr_el2, x0
	eret

el1_start:
	//Setting sp
	ldr x0, =EL1_SP
	mov sp, x0
	
	//Setting exception table
	ldr x0, =exception_table
	msr vbar_el1, x0

	bl irq_enable
	//Run main function
5:	b main
	b 5b
	