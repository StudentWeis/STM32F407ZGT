/**
 * @file w25qxx.c
 * @author StudentWei【studentweis@gmail.com】
 * @brief W25QXX 的驱动函数，适用于 W25Q16、W25Q32、W25Q64、W25Q128
 * @version 0.1
 * @date 2021-11-30
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "w25qxx.h"

/**
 * @brief W25Qxx 写指令
 *
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
 * @brief   读取 Flash 内部的 ID
 * @param   none
 * @retval  成功则返回 device_id
 */
uint16_t W25QXX_ReadID(void)
{
  uint16_t device_id = 0;
  uint8_t recv_buf[2] = {0};                                       // recv_buf[0]存放Manufacture ID, recv_buf[1]存放Device ID
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
 * @brief     读取W25QXX的状态寄存器，W25Q64一共有2个状态寄存器
 * @param     reg  —— 状态寄存器编号(1~2)
 * @retval    状态寄存器的值
 */
static uint8_t W25QXX_ReadSR(uint8_t reg)
{
  uint8_t result = 0;
  uint8_t send_buf[4] = {0x00, 0x00, 0x00, 0x00};
  switch (reg)
  {
  case 1:
    send_buf[0] = READ_STATU_REGISTER_1;
  case 2:
    send_buf[0] = READ_STATU_REGISTER_2;
  case 0:
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
 * @brief	阻塞等待Flash处于空闲状态
 * @param   none
 * @retval  none
 */
static void W25QXX_Wait_Busy(void)
{
  // 等待BUSY位清空
  while ((W25QXX_ReadSR(1) & 0x01) == 0x01)
    ;
}

/**
 * @brief   读取SPI FLASH数据
 * @param   buffer 数据存储区
 * @param   start_addr 开始读取的地址(最大32bit)
 * @param   nbytes_write 要读取的字节数(最大65535)
 * @retval  成功返回0，失败返回-1
 */
int W25QXX_Read(uint8_t *buffer, uint32_t start_addr, uint16_t nbytes_write)
{
  uint8_t cmd = READ_DATA_CMD;
  W25QXX_Wait_Busy();
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
 * @brief    页写入操作
 * @param    dat —— 要写入的数据缓冲区首地址
 * @param    WriteAddr —— 要写入的地址
 * @param    byte_to_write —— 要写入的字节数（0-256）
 * @retval   none
 */
void W25QXX_Page_Program(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes_write)
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
  W25QXX_Wait_Busy();
}

/**
 * @brief    写入操作
 * @param    dat 要写入的数据缓冲区首地址
 * @param    WriteAddr 要写入的地址
 * @param    nbytes_write
 * @retval   无
 * @note     突破 Page 限制
 */
void W25QXX_Write(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes_write)
{
  uint16_t nbytes_page_remain = 256 - (WriteAddr % 256); // 当前页剩余的能写入的字节数
  printf("num:%d\r\nnumremain:%d\r\n", nbytes_write, nbytes_page_remain);
  // 判断当前页是否能写完
  // 若能则直接写
  if (nbytes_write <= nbytes_page_remain)
  {
    W25QXX_Page_Program(dat, WriteAddr, nbytes_write);
  }
  else
  {
    W25QXX_Page_Program(dat, WriteAddr, nbytes_page_remain);
    dat += nbytes_page_remain;
    WriteAddr += nbytes_page_remain;
    nbytes_write -= nbytes_page_remain;
    W25QXX_Write(dat, WriteAddr, nbytes_write);
  }
}

/**
 * @brief   W25QXX 擦除一个扇区
 * @param   sector_addr    —— 扇区地址 根据实际容量设置
 * @retval  none
 */
void W25QXX_Erase_Sector(uint32_t sector_addr)
{
  uint8_t cmd = SECTOR_ERASE_CMD;
  sector_addr *= 4096; // 每个 Block 有 16 个 Sector，每个 Sector 的大小是 4KB，需要换算为实际字节地址
  printf("sector:%08x\r\n", sector_addr);

  W25QXX_CMD_WRITE_ENABLE();
  W25QXX_Wait_Busy(); // 等待写使能完成
  FLASH_SPI_CS_DOWN();
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF0000) >> 16;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF00) >> 8;
  FLASH_SPI_TRANSMIT(&cmd, 1);
  cmd = (sector_addr & 0xFF);
  FLASH_SPI_TRANSMIT(&cmd, 1);
  FLASH_SPI_CS_UP();
  W25QXX_Wait_Busy(); // 等待扇区擦除完成
}
