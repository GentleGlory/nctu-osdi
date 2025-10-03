#include "exception.h"


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
			b sync_exc_handler

		el\()LOWER_EL\()t_64_irq:
			b irq_exc_handler

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
		sub sp, sp, #272
		stp x0, x1, [sp, #16 * 0]
		stp x2, x3, [sp, #16 * 1]
		stp x4, x5, [sp, #16 * 2]
		stp x6, x7, [sp, #16 * 3]
		stp x8, x9, [sp, #16 * 4]
		stp x10, x11, [sp, #16 * 5]
		stp x12, x13, [sp, #16 * 6]
		stp x14, x15, [sp, #16 * 7]
		stp x16, x17, [sp, #16 * 8]
		stp x18, x19, [sp, #16 * 9]
		stp x20, x21, [sp, #16 * 10]
		stp x22, x23, [sp, #16 * 11]
		stp x24, x25, [sp, #16 * 12]
		stp x26, x27, [sp, #16 * 13]
		stp x28, x29, [sp, #16 * 14]
		str x30, [sp, #16 * 15]

		mrs x21, elr_el\()CURRENT_EL
		mrs x22, spsr_el\()CURRENT_EL
		stp x21, x22, [sp, #16 * 16]
	.endm

	.macro kernel_exit
		ldp x21, x22, [sp, #16 * 16]
		msr spsr_el\()CURRENT_EL, x22
		msr elr_el\()CURRENT_EL, x21

		ldr x30, [sp, #16 * 15]

		ldp x28, x29, [sp, #16 * 14]
		ldp x26, x27, [sp, #16 * 13]
		ldp x24, x25, [sp, #16 * 12]
		ldp x22, x23, [sp, #16 * 11]
		ldp x20, x21, [sp, #16 * 10]
		ldp x18, x19, [sp, #16 * 9]
		ldp x16, x17, [sp, #16 * 8]
		ldp x14, x15, [sp, #16 * 7]
		ldp x12, x13, [sp, #16 * 6]
		ldp x10, x11, [sp, #16 * 5]
		ldp x8, x9, [sp, #16 * 4]
		ldp x6, x7, [sp, #16 * 3]
		ldp x4, x5, [sp, #16 * 2]
		ldp x2, x3, [sp, #16 * 1]
		ldp x0, x1, [sp, #16 * 0]
		add sp, sp, #272
		eret
	.endm

	.macro DEFINE_SYNC_EXC_HANDLER
		sync_exc_handler:
			kernel_entry
			mrs x0, esr_el\()CURRENT_EL
			mrs x1, elr_el\()CURRENT_EL
			bl  sync_exc_router
			kernel_exit
	.endm

	.macro DEFINE_IRQ_EXC_HANDLER
		irq_exc_handler:
			kernel_entry
			bl  irq_exc_router
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
DEFINE_IRQ_EXC_HANDLER