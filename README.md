STM32F407ZGTx CubeMX 练习

### 任务1

测试 SDIO

参考：https://blog.csdn.net/qq_42039294/article/details/112045786

一次成功。

### 任务2

测试 FatFs

参考：https://www.cnblogs.com/showtime20190824/p/11523402.html

第一次失败：因为之前 SDIO 的学习中破坏了 SD 卡格式，所以需要先格式化再进行 FatFs 文件系统调用。格式化的文件系统是 FAT32（FatFs 默认）。
