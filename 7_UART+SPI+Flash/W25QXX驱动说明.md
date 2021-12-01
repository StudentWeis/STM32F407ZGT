使用 SPI 通信，需要提供 SPI 的发送接收接口函数，直接替换对应的宏定义即可。

- 函数：SPI 发送任意字节数据；
- 函数：SPI 接收任意字节数据；

将 FLASH 的读写函数封装，读取函数很简单，直接读就可以。写函数需要考虑 Page 的限制问题。



### 函数

- `W25QXX_Page_AnyByte`：从任意字节开始写，只能写单页；
- `W25QXX_nPages_AnyByte`：从任意字节开始写，连续跨页写；
- `W25QXX_nPages_PageFirstB`：从页首字节开始写，连续跨页写；
- `W25QXX_nPages_SectorFirstB`：从扇区首字节开始写，连续跨页写；
- `W25QXX_nPages_SectorClean`：擦除扇区，从扇区首字节开始写，连续跨页写；
- `W25QXX_Read`：从任意字节开始，读取任意数量字节；



### 宏定义

- `W25QXX_NPAGES_ANYBYTE_NUM`：从任意字节开始写，连续跨页写；不需要考虑字节数量参数；







由于 FLASH 的特性，需要在写之前将对应的 Sector 擦除。需要输入一个字节的地址，然后换算出其所在的 Sector。

- Byte2Sector

- Sector2Byte

  
