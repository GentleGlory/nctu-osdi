# Q&A
- Question 1：Is it reasonable to accelerate booting speed by parallel programming during the initialization stage?
	```
	在初始化的過程中，可能涉及一些全域性的設定。如設定 MMU 、 Stack Pointer等等。 所以只讓一個核心工作會是比較好的選擇。以免產生 race condition 。
	```
- Question 2:Point out the difference between bare-metal programming and programming on top of operating system.
	```
	裸機開發的過程需要自己去實作與底層硬體溝通，系統資源的控管。在有作業系統的環境下開發程式的話。這些作業系統都會幫你處理好。我們只要呼叫相關的API即可。

	```

# Note
## System core freq
- 如果 mini uart 輸出為亂碼的話。先確定 core freq 是否為 250 MHZ。 在 boot/config.txt 中加入以下的設定將 core freq 固定為 250 MHZ。
	```
	core_freq=250
	```