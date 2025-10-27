/*
 * This file generates assembly offset definitions for use in assembly code.
 * Based on Linux kernel's asm-offsets mechanism.
 */

#define __ASM_OFFSETS_C__

#include "task.h"
#include "exception.h"

/* Define macro to emit offset definitions */
#define DEFINE(sym, val) \
	asm volatile("\n#define " #sym " %0" : : "i" (val))

#define BLANK() \
	asm volatile("\n" : : )

int main(void)
{
	BLANK();
	DEFINE(CPU_CONTEXT_X19, offsetof(struct cpu_context, x19));
	DEFINE(CPU_CONTEXT_X20, offsetof(struct cpu_context, x20));
	DEFINE(CPU_CONTEXT_X21, offsetof(struct cpu_context, x21));
	DEFINE(CPU_CONTEXT_X22, offsetof(struct cpu_context, x22));
	DEFINE(CPU_CONTEXT_X23, offsetof(struct cpu_context, x23));
	DEFINE(CPU_CONTEXT_X24, offsetof(struct cpu_context, x24));
	DEFINE(CPU_CONTEXT_X25, offsetof(struct cpu_context, x25));
	DEFINE(CPU_CONTEXT_X26, offsetof(struct cpu_context, x26));
	DEFINE(CPU_CONTEXT_X27, offsetof(struct cpu_context, x27));
	DEFINE(CPU_CONTEXT_X28, offsetof(struct cpu_context, x28));
	DEFINE(CPU_CONTEXT_FP, offsetof(struct cpu_context, fp));
	DEFINE(CPU_CONTEXT_SP, offsetof(struct cpu_context, sp));
	DEFINE(CPU_CONTEXT_PC, offsetof(struct cpu_context, pc));
	BLANK();
	DEFINE(TASK_CPU_CONTEXT, offsetof(struct task, cpu_context));
	DEFINE(TASK_ID, offsetof(struct task, task_id));
	DEFINE(TASK_STATE, offsetof(struct task, state));
	DEFINE(RESERVED_USER_SP, offsetof(struct task, reserved_user_sp));
	DEFINE(RESERVED_KERNEL_SP, offsetof(struct task, reserved_kernel_sp));
	BLANK();

	DEFINE(S_X0, offsetof(struct pt_regs, regs[0]));
	DEFINE(S_X1, offsetof(struct pt_regs, regs[1]));
	DEFINE(S_X2, offsetof(struct pt_regs, regs[2]));
	DEFINE(S_X3, offsetof(struct pt_regs, regs[3]));
	DEFINE(S_X4, offsetof(struct pt_regs, regs[4]));
	DEFINE(S_X5, offsetof(struct pt_regs, regs[5]));
	DEFINE(S_X6, offsetof(struct pt_regs, regs[6]));
	DEFINE(S_X7, offsetof(struct pt_regs, regs[7]));
	DEFINE(S_X8, offsetof(struct pt_regs, regs[8]));
	DEFINE(S_X9, offsetof(struct pt_regs, regs[9]));
	DEFINE(S_X10, offsetof(struct pt_regs, regs[10]));
	DEFINE(S_X11, offsetof(struct pt_regs, regs[11]));
	DEFINE(S_X12, offsetof(struct pt_regs, regs[12]));
	DEFINE(S_X13, offsetof(struct pt_regs, regs[13]));
	DEFINE(S_X14, offsetof(struct pt_regs, regs[14]));
	DEFINE(S_X15, offsetof(struct pt_regs, regs[15]));
	DEFINE(S_X16, offsetof(struct pt_regs, regs[16]));
	DEFINE(S_X17, offsetof(struct pt_regs, regs[17]));
	DEFINE(S_X18, offsetof(struct pt_regs, regs[18]));
	DEFINE(S_X19, offsetof(struct pt_regs, regs[19]));
	DEFINE(S_X20, offsetof(struct pt_regs, regs[20]));
	DEFINE(S_X21, offsetof(struct pt_regs, regs[21]));
	DEFINE(S_X22, offsetof(struct pt_regs, regs[22]));
	DEFINE(S_X23, offsetof(struct pt_regs, regs[23]));
	DEFINE(S_X24, offsetof(struct pt_regs, regs[24]));
	DEFINE(S_X25, offsetof(struct pt_regs, regs[25]));
	DEFINE(S_X26, offsetof(struct pt_regs, regs[26]));
	DEFINE(S_X27, offsetof(struct pt_regs, regs[27]));
	DEFINE(S_X28, offsetof(struct pt_regs, regs[28]));
	DEFINE(S_FP, offsetof(struct pt_regs, regs[29]));
	DEFINE(S_LR, offsetof(struct pt_regs, regs[30]));
	DEFINE(S_ELR, offsetof(struct pt_regs, elr));
	DEFINE(S_SPSR, offsetof(struct pt_regs, spsr));
	DEFINE(S_USER_SP, offsetof(struct pt_regs, sp));
	DEFINE(PT_REGS_SIZE, sizeof(struct pt_regs));
	
	return 0;
}
