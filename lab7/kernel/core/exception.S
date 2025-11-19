#include "exception.h"
#include "sys_regs.h"
#include "asm-offsets.h"
#include "mmu.h"

	.macro kernel_ventry, el:req, ht:req, regsize:req, label:req
		.align 7
		b	el\el\ht\()_\regsize\()_\label
	.endm

	.macro handle_invalid_entry type
		mov x0, #\type
		mrs x1, esr_el\()CURRENT_EL
		mrs x2, elr_el\()CURRENT_EL
		bl	show_exception_status
		b	not_implemented
    .endm

	// Assembly macro to define kernel exception handlers
	.macro DEFINE_CURRENT_EL_EXCEPTION_HANDLERS
		el\()CURRENT_EL\()t_64_sync:
			handle_invalid_entry	SYNC_INVALID_CURRENT_ELt
			
		el\()CURRENT_EL\()t_64_irq:
			handle_invalid_entry	IRQ_INVALID_CURRENT_ELt

		el\()CURRENT_EL\()t_64_fiq:
			handle_invalid_entry	FIQ_INVALID_CURRENT_ELt

		el\()CURRENT_EL\()t_64_error:
			handle_invalid_entry	ERROR_INVALID_CURRENT_ELt

		el\()CURRENT_EL\()h_64_sync:
			b sync_exc_handler

		el\()CURRENT_EL\()h_64_irq:
			b irq_exc_handler

		el\()CURRENT_EL\()h_64_fiq:
			handle_invalid_entry	FIQ_INVALID_CURRENT_ELh

		el\()CURRENT_EL\()h_64_error:
			handle_invalid_entry	ERROR_INVALID_CURRENT_ELh
	.endm

	// Assembly macro to define kernel exception handlers
	.macro DEFINE_LOWER_EL_EXCEPTION_HANDLERS
		el\()LOWER_EL\()t_64_sync:
			b lower_el_sync_exc_handler

		el\()LOWER_EL\()t_64_irq:
			b irq_exc_el\()LOWER_EL\()_handler

		el\()LOWER_EL\()t_64_fiq:
			handle_invalid_entry	FIQ_INVALID_LOWER_EL_64

		el\()LOWER_EL\()t_64_error:
			handle_invalid_entry	ERROR_INVALID_LOWER_EL_64

		el\()LOWER_EL\()t_32_sync:
			handle_invalid_entry	SYNC_INVALID_LOWER_EL_32

		el\()LOWER_EL\()t_32_irq:
			handle_invalid_entry	IRQ_INVALID_LOWER_EL_32

		el\()LOWER_EL\()t_32_fiq:
			handle_invalid_entry	FIQ_INVALID_LOWER_EL_32

		el\()LOWER_EL\()t_32_error:
			handle_invalid_entry	ERROR_INVALID_LOWER_EL_32
	.endm

	.macro kernel_entry
		sub sp, sp, #PT_REGS_SIZE
		stp x0, x1, [sp, #S_X0]
		stp x2, x3, [sp, #S_X2]
		stp x4, x5, [sp, #S_X4]
		stp x6, x7, [sp, #S_X6]
		stp x8, x9, [sp, #S_X8]
		stp x10, x11, [sp, #S_X10]
		stp x12, x13, [sp, #S_X12]
		stp x14, x15, [sp, #S_X14]
		stp x16, x17, [sp, #S_X16]
		stp x18, x19, [sp, #S_X18]
		stp x20, x21, [sp, #S_X20]
		stp x22, x23, [sp, #S_X22]
		stp x24, x25, [sp, #S_X24]
		stp x26, x27, [sp, #S_X26]
		stp x28, x29, [sp, #S_X28]

		mrs x21, elr_el\()CURRENT_EL
		mrs x22, spsr_el\()CURRENT_EL
		mrs x23, sp_el\()LOWER_EL
		
		stp x30, x21, [sp, #S_LR]
		stp x22, x23, [sp, #S_SPSR]

		// Ensure TTBR0 points to the kernel identity map so MMIO is accessible
		ldr x24, =__id_map_pgd_start
		ldr x25, =__kernel_virtual_base
		sub x24, x24, x25
		dsb ish
		msr ttbr0_el1, x24
		tlbi vmalle1is
		dsb ish
		isb
	.endm

	.macro kernel_exit
		mrs x0, tpidr_el1					//get current task
		ldr x1, [x0, #USER_PGD_PAGE]
		ldr x2, =__kernel_virtual_base
		cbnz x1, 1f 
		ldr x1, =__id_map_pgd_start		
		sub x1, x1, x2
		b 2f
1:
		sub x1, x1, x2		
2:	
		dsb ish				// ensure write has completed
		msr ttbr0_el1, x1	// switch translation based address.
		tlbi vmalle1is		// invalidate all TLB entries
		dsb ish				// ensure completion of TLB invalidatation
		isb 				// clear pipeline
				
		ldp x22, x23, [sp, #S_SPSR]
		ldp x30, x21, [sp, #S_LR]
		
		msr elr_el\()CURRENT_EL, x21
		msr spsr_el\()CURRENT_EL, x22
		msr sp_el\()LOWER_EL, x23

		ldp x28, x29, [sp, #S_X28]
		ldp x26, x27, [sp, #S_X26]
		ldp x24, x25, [sp, #S_X24]
		ldp x22, x23, [sp, #S_X22]
		ldp x20, x21, [sp, #S_X20]
		ldp x18, x19, [sp, #S_X18]
		ldp x16, x17, [sp, #S_X16]
		ldp x14, x15, [sp, #S_X14]
		ldp x12, x13, [sp, #S_X12]
		ldp x10, x11, [sp, #S_X10]
		ldp x8, x9, [sp, #S_X8]
		ldp x6, x7, [sp, #S_X6]
		ldp x4, x5, [sp, #S_X4]
		ldp x2, x3, [sp, #S_X2]
		ldp x0, x1, [sp, #S_X0]
		add sp, sp, #PT_REGS_SIZE
		
		eret
	.endm

	.macro DEFINE_SYNC_EXC_HANDLER
		.align	3
		sync_exc_handler:
			kernel_entry
			mov x0, sp
			bl  el\()CURRENT_EL\()_sync_exc_router
			kernel_exit
	.endm

	.macro DEFINE_LOWER_EL_SYNC_EXC_HANDLER
		.align	3
		lower_el_sync_exc_handler:
			kernel_entry
			mov x0, sp
			bl  el\()LOWER_EL\()_sync_exc_router
			kernel_exit
	.endm

	.macro DEFINE_IRQ_EXC_HANDLER
		.align	3
		irq_exc_handler:
			kernel_entry
			bl  irq_exc_router
			bl  scheduler_do_schedule
			kernel_exit
	.endm

	.macro RET_TO_USER
		.global ret_to_user
		.align	3		
		ret_to_user:
			sub sp, sp, #16
			
			str x0, [sp, #0]					//store the function pointer
			
			bl lock_irq_save
			
			mov x2, x0
			
			mrs x0, tpidr_el1					//get current task
			ldr x21, [sp, #0]					//get the function pointer
			ldr x22, =SPSR_EL1_EL0
			ldr x23, [x0, #RESERVED_USER_SP]
		
			ldr x1, [x0, #RESERVED_KERNEL_SP]	//get the kernel sp
			mov sp, x1

			sub sp, sp, #PT_REGS_SIZE
			stp xzr, xzr, [sp, #S_X0]
			stp xzr, xzr, [sp, #S_X2]
			stp xzr, xzr, [sp, #S_X4]
			stp xzr, xzr, [sp, #S_X6]
			stp xzr, xzr, [sp, #S_X8]
			stp xzr, xzr, [sp, #S_X10]
			stp xzr, xzr, [sp, #S_X12]
			stp xzr, xzr, [sp, #S_X14]
			stp xzr, xzr, [sp, #S_X16]
			stp xzr, xzr, [sp, #S_X18]
			stp xzr, xzr, [sp, #S_X20]
			stp xzr, xzr, [sp, #S_X22]
			stp xzr, xzr, [sp, #S_X24]
			stp xzr, xzr, [sp, #S_X26]
			stp xzr, xzr, [sp, #S_X28]
			stp xzr, x21, [sp, #S_LR]
			stp x22, x23, [sp, #S_SPSR]
			
			mov x0, x2
			bl lock_irq_restore

			kernel_exit
	.endm

	.macro DEFINE_LOW_LEVEL_IRQ_EXC_HANDLER
		.align	3
		irq_exc_el\()LOWER_EL\()_handler:
			kernel_entry
			bl irq_exc_router
			bl scheduler_do_schedule
			kernel_exit
	.endm

.section ".text"
.global exception_table

.align	11
exception_table:
	kernel_ventry	CURRENT_EL, t, 64, sync			// Synchronous ELxt
	kernel_ventry	CURRENT_EL, t, 64, irq			// IRQ ELxt
	kernel_ventry	CURRENT_EL, t, 64, fiq			// FIQ ELxt
	kernel_ventry	CURRENT_EL, t, 64, error			// Error ELxt

	kernel_ventry	CURRENT_EL, h, 64, sync			// Synchronous ELxh
	kernel_ventry	CURRENT_EL, h, 64, irq			// IRQ ELxh
	kernel_ventry	CURRENT_EL, h, 64, fiq			// FIQ ELxh
	kernel_ventry	CURRENT_EL, h, 64, error			// Error ELxh

	kernel_ventry	LOWER_EL, t, 64, sync		// Synchronous 64-bit ELx
	kernel_ventry	LOWER_EL, t, 64, irq		// IRQ 64-bit ELx
	kernel_ventry	LOWER_EL, t, 64, fiq		// FIQ 64-bit ELx
	kernel_ventry	LOWER_EL, t, 64, error		// Error 64-bit ELx

	kernel_ventry	LOWER_EL, t, 32, sync		// Synchronous 32-bit ELx
	kernel_ventry	LOWER_EL, t, 32, irq		// IRQ 32-bit ELx
	kernel_ventry	LOWER_EL, t, 32, fiq		// FIQ 32-bit ELx
	kernel_ventry	LOWER_EL, t, 32, error		// Error 32-bit ELx

// Generate exception handlers using the macro
DEFINE_CURRENT_EL_EXCEPTION_HANDLERS
DEFINE_LOWER_EL_EXCEPTION_HANDLERS

DEFINE_SYNC_EXC_HANDLER
DEFINE_LOWER_EL_SYNC_EXC_HANDLER
DEFINE_IRQ_EXC_HANDLER
RET_TO_USER

DEFINE_LOW_LEVEL_IRQ_EXC_HANDLER
