# Q&A
- Question 1：What’s the RAM size of Raspberry Pi 3B+?
	```
	從官網上看應該為 1GB LPDDR2 SDRAM。
	```
	[Raspberry Pi 3B+ Spec](https://www.raspberrypi.com/products/raspberry-pi-3-model-b-plus/)

- Question 2：What’s the cache size and level of Raspberry Pi 3B+?
	```
	由lspcu的結果可以知道為：有四個核心，每個核心有 128 KB 的 Level 1 data cache。 128 KB 的 Level 1 instruction cache。以及一個所有核心共用的 512 KB Level 2 cache。
	```
	```shell
	frank@raspberrypi:~ $ lscpu
	Architecture:                aarch64
	  CPU op-mode(s):            32-bit, 64-bit
	  Byte Order:                Little Endian
	CPU(s):                      4
	  On-line CPU(s) list:       0-3
	Vendor ID:                   ARM
	  Model name:                Cortex-A53
	    Model:                   4
	    Thread(s) per core:      1
	    Core(s) per cluster:     4
	    Socket(s):               -
	    Cluster(s):              1
	    Stepping:                r0p4
	    CPU(s) scaling MHz:      100%
	    CPU max MHz:             1400.0000
	    CPU min MHz:             600.0000
	    BogoMIPS:                38.40
	    Flags:                   fp asimd evtstrm crc32 cpuid
	Caches (sum of all):
	  L1d:                       128 KiB (4 instances)
	  L1i:                       128 KiB (4 instances)
	  L2:                        512 KiB (1 instance)
	NUMA:
	  NUMA node(s):              1
	  NUMA node0 CPU(s):         0-3
	Vulnerabilities:
	  Gather data sampling:      Not affected
	  Indirect target selection: Not affected
	  Itlb multihit:             Not affected
	  L1tf:                      Not affected
	  Mds:                       Not affected
	  Meltdown:                  Not affected
	  Mmio stale data:           Not affected
	  Reg file data sampling:    Not affected
	  Retbleed:                  Not affected
	  Spec rstack overflow:      Not affected
	  Spec store bypass:         Not affected
	  Spectre v1:                Mitigation; __user pointer sanitization
	  Spectre v2:                Not affected
	  Srbds:                     Not affected
	  Tsx async abort:           Not affected
	```
- Question 3: Explain each line of the below linker script.
	```
	SECTIONS //定義輸出檔的段落格局
	{
	  . = 0x80000; //將 location counter 當前位置設定為 0x80000
	  .text : { *(.text) } //建立一個輸出段 .text。並將obj檔內的 .text 段都放在裡面。
	}
	```

# Note
## 如何用 gdb & qemu Debug
```shell
$ make qemu_debug

開啟另一個terminal
$ gdb-multiarch build/kernel8.elf

在gdb底下
(gdb) target remote :1234       # 連到 QEMU
(gdb) set architecture aarch64  # 指定 CPU 架構
(gdb) break _start              # 在 kernel 入口 (假設有符號)
(gdb) continue                  # 開始執行

常用指令
(gdb) info registers      # 查看暫存器
(gdb) x/10i $pc           # 反組譯程式計數器附近的指令
(gdb) x/16x 0x80000       # 查看 0x80000 記憶體內容
(gdb) stepi               # 單步執行一條指令
(gdb) backtrace           # 呼叫堆疊 (需要符號)

```
