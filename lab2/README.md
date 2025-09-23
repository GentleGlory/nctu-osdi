# Q&A
- Question 1: In x86 machine, how the above 4 steps are implemented? Roughly describe it.
```
1. 系統上電後執行 BIOS / UEFI。 BIOS / UEFI 確認記憶體、顯示卡等基本硬體是否存在並能正常運作。如果有問題會透過蜂鳴聲通知或是顯示 Error Code 。
2. 確認硬體沒有問題後， BIOS / UEFI 就會按照啟動順序去尋找可以開機的裝置，然後載入 bootloader。
3. bootloader 再載入 kernel 。
```

# Note
