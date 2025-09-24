# Q&A
- Question 1：Change svc instruction to brk (breakpoint) instruction. See the difference in ELR_EL2(return address). Explain why there is a difference.
	```
	系統一樣會進到 sync_exc_router 裡面。和 svc 的差別是，ELR_EL2 指向觸發指令本身，而不是下一條指令。所以直接 eret ， CPU 會重執行 brk → 又例外 → 進入死循環。
	```
- Question 2: Do you need to save floating point SIMD registers in ISRs? Why or why not.
	```
	這要取決於 ISR 會不會使用到 SIMD registers 。 如果不會，那就不需要保存。這樣會加快整個中斷處理的速度。 如果 ISR 會使用到 SIMD registers 的話那就需要保存。
	```
- Question 3: What will happen if you don’t clear peripherals’ interrupt signal?
	```
	如果不清掉的話，那會持續觸發中斷。產生中斷風暴。
	```
# Note
