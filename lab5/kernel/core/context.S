#include "asm-offsets.h"

.section ".text"
.align	3
.global switch_to
switch_to:
	stp x19, x20, [x0, #CPU_CONTEXT_X19]
	stp x21, x22, [x0, #CPU_CONTEXT_X21]
	stp x23, x24, [x0, #CPU_CONTEXT_X23]
	stp x25, x26, [x0, #CPU_CONTEXT_X25]
	stp x27, x28, [x0, #CPU_CONTEXT_X27]
	str fp, [x0, #CPU_CONTEXT_FP]
	mov x9, sp
	str x9, [x0, #CPU_CONTEXT_SP]
	str lr, [x0, #CPU_CONTEXT_PC]
	
	ldp x19, x20, [x1, #CPU_CONTEXT_X19]
	ldp x21, x22, [x1, #CPU_CONTEXT_X21]
	ldp x23, x24, [x1, #CPU_CONTEXT_X23]
	ldp x25, x26, [x1, #CPU_CONTEXT_X25]
	ldp x27, x28, [x1, #CPU_CONTEXT_X27]
	ldr fp, [x1, #CPU_CONTEXT_FP]
	ldr x9, [x1, #CPU_CONTEXT_SP]
	mov sp, x9
	ldr lr, [x1, #CPU_CONTEXT_PC]
	
	msr tpidr_el1, x1
	cbnz x2, 1f
	msr daifclr, #2
1:
	ret

.global get_current
.align	3
get_current:
	mrs x0, tpidr_el1
	ret

.global set_current
.align	3
set_current:
	msr tpidr_el1, x0
	ret
