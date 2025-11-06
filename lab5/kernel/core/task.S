.section ".text"
.global task_do_fork
.align	3
//0: return_address
task_do_fork:
	sub sp, sp, #16
	str lr, [sp, #0]
	
	mov x1, sp
	adrp x2, child_return
	add x2, x2, #:lo12:child_return

	bl task_prepare_fork

child_return:
	
	ldr lr, [sp, #0]
	add sp, sp, #16
	ret
