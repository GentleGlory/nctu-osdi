/*
 * This file generates assembly offset definitions for use in assembly code.
 * Based on Linux kernel's asm-offsets mechanism.
 */

#define __ASM_OFFSETS_C__

#include "task.h"

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
	BLANK();

	return 0;
}
