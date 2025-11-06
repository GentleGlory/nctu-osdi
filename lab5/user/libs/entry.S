
.section .text
.global _start
.align 3

_start:
	bl main		// call main()
				// return value will be in x0
	bl exit		// call exit(x0)
				// never returns
