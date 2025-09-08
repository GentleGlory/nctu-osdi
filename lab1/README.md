# Q&A
- Question 1：Is it reasonable to accelerate booting speed by parallel programming during the initialization stage?
	```
	在初始化的過程中，可能涉及一些全域性的設定。如設定 MMU 、 Stack Pointer等等。 所以只讓一個核心工作會是比較好的選擇。以免產生 race condition 。
	```