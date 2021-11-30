#ifndef __W25QXX_H__
#define __W25QXX_H__

#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include <string.h>

// FLASH-SPI 接口宏定义
#define FLASH_SPI_CS_DOWN() HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_RESET)
#define FLASH_SPI_CS_UP() HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_SET)
#define FLASH_SPI_TRANSMIT(buf, size) HAL_SPI_Transmit(&hspi1, buf, size, 100)
#define FLASH_SPI_RECEIVE(buf, size) HAL_SPI_Receive(&hspi1, buf, size, 100)
#define FLASH_SPI_T_R(sbuf, rbuf, size) HAL_SPI_TransmitReceive(&hspi1, sbuf, rbuf, size, 100);

// W25Qxx 指令宏定义
#define ManufactDeviceID_CMD 0x90
#define READ_STATU_REGISTER_1 0x05
#define READ_STATU_REGISTER_2 0x35
#define READ_DATA_CMD 0x03
#define WRITE_ENABLE_CMD 0x06
#define WRITE_DISABLE_CMD 0x04
#define SECTOR_ERASE_CMD 0x20
#define CHIP_ERASE_CMD 0xc7
#define PAGE_PROGRAM_CMD 0x02

// W25Qxx 操作宏定义
#define W25QXX_CMD_WRITE_ENABLE() W25QXX_WriteCMD(WRITE_ENABLE_CMD)

// W25Qxx 私有函数声明
static void W25QXX_WriteCMD(uint8_t PCMD);
static uint8_t W25QXX_ReadSR(uint8_t reg);
static void W25QXX_Wait_Busy(void);

// W25Qxx 外界函数声明
uint16_t W25QXX_ReadID(void);
int W25QXX_Read(uint8_t *buffer, uint32_t start_addr, uint16_t nbytes);
void W25QXX_Page_Program(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes);
void W25QXX_Erase_Sector(uint32_t sector_addr);
void W25QXX_Write(uint8_t *dat, uint32_t WriteAddr, uint16_t nbytes_write);


#endif //__W25QXX_H__
