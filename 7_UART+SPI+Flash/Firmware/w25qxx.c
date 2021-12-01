/**
 * @file w25qxx.c
 * @author StudentWei【studentweis@gmail.com】
 * @brief W25QXX 的驱动源程序，适用于 W25Q16、W25Q32、W25Q64、W25Q128，留有 SPI 接口，可方便地在不同平台之间迁移。
 * @version 0.1
 * @date 2021-12-01
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "w25qxx.h"

/* 私有函数定义 */

/**
 * @brief W25Qxx 写指令
 * @param PCMD 指令
 */
static void W25QXX_WriteCMD(uint8_t PCMD)
{
  uint8_t CMD[1];
  CMD[0] = PCMD;
  FLASH_SPI_CS_DOWN();
  FLASH_SPI_TRANSMIT(CMD, 1);
  FLASH_SPI_CS_UP();
}

/**
 * @brief     读取 W25QXX 的 Read 状态寄存器，W25QXX 一共有 3 个 Read 状态寄存器
 * @param     reg 读状态寄存器编号(1~3)
 * @retval    读状态寄存器的值
 */
static uint8_t W25QXX_ReadRSR(uint8_t reg)
{
  uint8_t result = 0;
  uint8_t send_buf[4] = {0x00, 0x00, 0x00, 0x00};
  switch (reg)
  {
  case 1:
    send_buf[0] = READ_STATU_REGISTER_1;
  case 2:
    send_buf[0] = READ_STATU_REGISTER_2;
  case 3:
    send_buf[0] = READ_STATU_REGISTER_3;
  default:
    send_buf[0] = READ_STATU_REGISTER_1;
  }
  FLASH_SPI_CS_DOWN();
  if (HAL_OK == FLASH_SPI_TRANSMIT(send_buf, 4))
  {
    if (HAL_OK == FLASH_SPI_RECEIVE(&result, 1))
    {
      FLASH_SPI_CS_UP();
      return result;
    }
  }
  FLASH_SPI_CS_UP();
  return 0;
}

/**
 * @brief   读取 Flash 内部的 ID
 * @param   none
 * @retval  成功则返回 device_id
 */
uint16_t W25QXX_ReadID(void)
{
  uint16_t device_id = 0;
  uint8_t recv_buf[2] = {0};                                       // recv_buf[0] 存放 Manufacture ID, recv_buf[1] 存放 Device ID
  uint8_t send_data[4] = {ManufactDeviceID_CMD, 0x00, 0x00, 0x00}; //待发送数据，命令+地址

  FLASH_SPI_CS_DOWN();
  /* 发送并读取数据 */
  if (HAL_OK == FLASH_SPI_TRANSMIT(send_data, 4))
  {
    if (HAL_OK == FLASH_SPI_RECEIVE(recv_buf, 2))
    {
      device_id = (recv_buf[0] << 8) | recv_buf[1];
    }
  }
  FLASH_SPI_CS_UP();
  return device_id;
}

/**
 * @brief    页写入操作
 * @param    dat —— 要写入的数据缓冲区首地址
 * @param    WriteAddr —— 要写入的地址
 * @param    byte_to_write —— 要写入的字节数（0-256）
 * @retval   none
 */
static void W25QXX_Page_Program(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes_write)
{
  uint8_t cmd = PAGE_PROGRAM_CMD;
  W25QXX_CMD_WRITE_ENABLE();
  FLASH_SPI_CS_DOWN();
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (WriteAddr & 0xFF0000) >> 16;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (WriteAddr & 0xFF00) >> 8;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (WriteAddr & 0xFF);
  FLASH_SPI_TRANSMIT(&cmd, 1);
  FLASH_SPI_TRANSMIT(dat, nbytes_write);
  FLASH_SPI_CS_UP();
  W25QXX_WAIT_BUSY();
}

/**
 * @brief   W25QXX 擦除一个扇区
 * @param   sector_addr 扇区地址 根据实际容量设置
 * @retval  none
 */
void W25QXX_Erase_Sector(uint32_t sector_addr)
{
  uint8_t cmd = SECTOR_ERASE_CMD;
  sector_addr *= 4096; // 每个 Sector 的大小是 4KB，需要换算为实际字节地址

  W25QXX_CMD_WRITE_ENABLE();
  W25QXX_WAIT_BUSY(); // 等待写使能完成
  FLASH_SPI_CS_DOWN();
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF0000) >> 16;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF00) >> 8;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF);
  FLASH_SPI_TRANSMIT(&cmd, 1);
  FLASH_SPI_CS_UP();
  W25QXX_WAIT_BUSY(); // 等待扇区擦除完成
}

/**
 * @brief   读取 SPI FLASH 数据
 * @param   buffer 数据存储区
 * @param   start_addr 开始读取的地址(最大 32bit)
 * @param   nbytes_write 要读取的字节数(最大 65535)
 * @retval  成功返回0，失败返回-1
 */
int W25QXX_Read(uint8_t *buffer, uint32_t start_addr, uint16_t nbytes_write)
{
  uint8_t cmd = READ_DATA_CMD;

  W25QXX_WAIT_BUSY();
  FLASH_SPI_CS_DOWN();
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (start_addr & 0xFF0000) >> 16;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (start_addr & 0xFF00) >> 8;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (start_addr & 0xFF);
  FLASH_SPI_TRANSMIT(&cmd, 1);
  if (HAL_OK == FLASH_SPI_RECEIVE(buffer, nbytes_write))
  {
    FLASH_SPI_CS_UP();
    return 0;
  }
  FLASH_SPI_CS_UP();
  return -1;
}

/**
 * @brief    写入操作
 * @param    dat 要写入的数据缓冲区首地址
 * @param    WriteAddr 要写入的地址
 * @param    nbytes_write  要写入的字节数（0-65535）
 * @retval   无
 * @note     突破 Page 限制
 */
void W25QXX_Pages_Write(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes_write)
{
  uint16_t nbytes_page_remain = 256 - (WriteAddr % 256); // 计算当前页剩余的能写入的字节数
#ifdef MY_DEBUG
  printf("num:%d\r\nnumremain:%d\r\n", nbytes_write, nbytes_page_remain);
#endif
  /* 判断当前页是否能写完 */
  // 若能则直接写
  if (nbytes_write <= nbytes_page_remain)
  {
    W25QXX_Page_Program(dat, WriteAddr, nbytes_write);
  }
  // 否则先写一部分，然后递归调用
  else
  {
    W25QXX_Page_Program(dat, WriteAddr, nbytes_page_remain);
    dat += nbytes_page_remain;
    WriteAddr += nbytes_page_remain;
    nbytes_write -= nbytes_page_remain;
    W25QXX_Pages_Write(dat, WriteAddr, nbytes_write);
  }
}
