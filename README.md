STM32F407ZGTx CubeMX 练习

Firmware 版本：1.26.2

### 任务1

测试 SDIO

参考：https://blog.csdn.net/qq_42039294/article/details/112045786

一次成功。

### 任务2

测试 FatFs

参考：https://www.cnblogs.com/showtime20190824/p/11523402.html

第一次失败：因为之前 SDIO 的学习中破坏了 SD 卡格式，所以需要先格式化再进行 FatFs 文件系统调用。格式化的文件系统是 FAT32（FatFs 默认）。

### 任务3

测试 RTC

核心板硬件上装有低速 32kHz 晶振，且装有纽扣电池，可以进行测试。

主要目标：

1. 基本时钟使用；
2. 掉电继续运行；
3. 关闭掉电继续运行模式；

参考：https://blog.csdn.net/as480133937/article/details/105741893?spm=1001.2014.3001.5501

- 时钟选择要选择 LSE，不要用默认的 LSI，这样掉电之后才有时钟。

- 分频寄存器是自动配置的，为了凑够 1s 的中断。

- 写的时候 BCD 写，读的时候 BIN 读，方便后续的显示。

- 重启之后 RTC 也重启了，如何避免？

  核心板需要连接 VBAT 备用电源；然后使用备份寄存器（Backup Registers）做记录，上电之后检查备份寄存器，然后跳过 RTC 初始化。

    ```c
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x5051)
    {
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5051);
        //....
    }
    else
    {
    }
    ```
  
- 掉电之后的机制到底是什么？

  VBat：数据手册 P31

  Power supply overview：参考手册 P116

  LSE：参考手册 P156

  RTC：参考手册 P221

  RCC Backup domain control register (RCC_BDCR)：参考手册 P199，这些不会被 Reset 复位。

  断电之后其他时钟就停了，而备用纽扣电池从 Vbat 口供电，维持 BKP 寄存器的数据、维持 LSE 时钟，为完全独立的 RTC 提供频率和电源，维持时钟继续。

- 如何关闭电池？为了节能。

  换回 LSI 时钟就行，默认不开启 LSE 即可。



