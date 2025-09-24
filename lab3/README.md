# Q&A
- Question 1：Change svc instruction to brk (breakpoint) instruction. See the difference in ELR_EL2(return address). Explain why there is a difference.
	```
	系統一樣會進到 sync_exc_router 裡面。和 svc 的差別是，ELR_EL2 指向觸發指令本身，而不是下一條指令。所以直接 eret ， CPU 會重執行 brk → 又例外 → 進入死循環。
	```
# Note
