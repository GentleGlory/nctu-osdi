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
- elective 3: Use a long delay to simulate bottom half of ISR.Compare the difference between enabling and not enabling interrupt after top half of ISR.
	
	```
	如果在 top half of ISR 之後不 Enabling interrupt. 那其他的 irq 就要等
	irq_long_delay_test 結束後才能執行。這樣會降低效率。

	//Not Enabling interrupt after top half of ISR.
	# irq
	System timer jiffies:1
	System timer jiffies:2
	System timer jiffies:3
	System timer jiffies:4
	System timer jiffies:5
	Core timer jiffies:1
	In irq_long_delay_test
	Out irq_long_delay_test
	System timer jiffies:6
	System timer jiffies:7
	System timer jiffies:8
	System timer jiffies:9
	System timer jiffies:10
	System timer jiffies:11
	Core timer jiffies:2
	In irq_long_delay_test
	Out irq_long_delay_test
	System timer jiffies:12
	System timer jiffies:13
	
	//Enabling interrupt after top half of ISR.
	# irq
	System timer jiffies:1
	System timer jiffies:2
	System timer jiffies:3
	System timer jiffies:4
	System timer jiffies:5
	Core timer jiffies:1
	In irq_long_delay_test
	System timer jiffies:6
	Out irq_long_delay_test
	System timer jiffies:7
	System timer jiffies:8
	System timer jiffies:9
	System timer jiffies:10
	System timer jiffies:11
	Core timer jiffies:2
	In irq_long_delay_test
	Out irq_long_delay_test
	System timer jiffies:12
	System timer jiffies:13
	System timer jiffies:14
	System timer jiffies:15
	System timer jiffies:16
	Core timer jiffies:3
	In irq_long_delay_test
	System timer jiffies:17
	Out irq_long_delay_test
	System timer jiffies:18
	System timer jiffies:19
	System timer jiffies:20
	System timer jiffies:21

	```
# Note
