/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void W25Q16_WriteCMD(uint8_t PCMD);
static uint8_t W25QXX_ReadSR(uint8_t reg);
static void W25QXX_Wait_Busy(void);
int W25QXX_Read(uint8_t* buffer, uint32_t start_addr, uint16_t nbytes);
void W25QXX_Page_Program(uint8_t* dat, uint32_t WriteAddr, uint16_t nbytes);
void W25QXX_Erase_Sector(uint32_t sector_addr);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ManufactDeviceID_CMD		0x90
#define READ_STATU_REGISTER_1   0x05
#define READ_STATU_REGISTER_2   0x35
#define READ_DATA_CMD	      	  0x03
#define WRITE_ENABLE_CMD	   	  0x06
#define WRITE_DISABLE_CMD	   	  0x04
#define SECTOR_ERASE_CMD	  	  0x20
#define CHIP_ERASE_CMD	        0xc7
#define PAGE_PROGRAM_CMD        0x02


#define FLASH_SPI_CS_DOWN()				HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_RESET)
#define FLASH_SPI_CS_UP()					HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_SET)

#define SPI1_TRANSMIT(buf, size) 	HAL_SPI_Transmit(&hspi1, buf, size, 100)
#define SPI1_RECEIEVE(buf, size) 	HAL_SPI_Receive(&hspi1, buf, size, 100)



#define W25Q16_WRITE_ENABLE()		W25Q16_WriteCMD(0x06)
#define W25Q16_WRITE()					W25Q16_WriteCMD(0x02)
#define W25Q16_READ()						W25Q16_WriteCMD(0x03)
#define W25Q16_ERASE_CHIP()			{W25Q16_WRITE_ENABLE(); W25Q16_WriteCMD(0x60);}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t read_buf[10] = {0};
uint8_t write_buf[10] = {0};
int i;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t ID[3];
	uint8_t CMD_ID[1] = {0x9f};

	

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	printf("This is the exercise by WEI\r\n");
	FLASH_SPI_CS_DOWN();
	HAL_SPI_Transmit(&hspi1, CMD_ID, 1, 1000);
	HAL_SPI_Receive(&hspi1, ID, 3, 1000);
	FLASH_SPI_CS_UP();
	printf("W25Qxxx ID is : %02X0x%02X%02X \r\n\r\n", ID[0], ID[1], ID[2]);
	
	
	 /* 为了验证，首先读取要写入地址处的数据 */
  printf("-------- read data before write -----------\r\n");
  W25QXX_Read(read_buf, 0, 10);
    
  for(i = 0; i < 10; i++) 
  {
    printf("[0x%08x]:0x%02x\r\n", i, *(read_buf+i));
  }
    
  /* 擦除该扇区 */
  printf("-------- erase sector 0 -----------\r\n");
  W25QXX_Erase_Sector(0);

  /* 再次读数据 */
  printf("-------- read data after erase -----------\r\n");
  W25QXX_Read(read_buf, 0, 10);
  for(i = 0; i < 10; i++) 
  {
    printf("[0x%08x]:0x%02x\r\n", i, *(read_buf+i));
  }
    
  /* 写数据 */
  printf("-------- write data -----------\r\n");
  for(i = 0; i < 10; i++) 
  {
    write_buf[i] = i;
  }
  W25QXX_Page_Program(write_buf, 0, 10);
    
  /* 再次读数据 */
  printf("-------- read data after write -----------\r\n");
  W25QXX_Read(read_buf, 0, 10);
  for(i = 0; i < 10; i++) 
  {
    printf("[0x%08x]:0x%02x\r\n", i, *(read_buf+i));
  }
	
//	for(uint16_t i; i < 256; i++)
//	{
//		DATA_TBuf[i] = i;
//	}
//	printf("TPrint:%d\r\n", DATA_TBuf[146]);
//	printf("RPrint0:%d\r\n", DATA_RBuf[146]);
//	
//	W25Q16_ERASE_CHIP();
//	W25QXX_Wait_Busy();
//	
//	W25QXX_Page_Program(DATA_TBuf, 0, 256);
//	W25QXX_Wait_Busy();
//	
//	W25QXX_Read(DATA_RBuf, 0, 256);
//	W25QXX_Wait_Busy();
//	printf("RPrint1:%d\r\n", DATA_RBuf[146]);

	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
  uint8_t temp[1] = {ch};
  HAL_UART_Transmit(&huart1, temp, 1, 2);
  return ch;
}

// W25Q16 写指令函数
void W25Q16_WriteCMD(uint8_t PCMD)
{
	uint8_t CMD[1];
	CMD[0] = PCMD;
	FLASH_SPI_CS_DOWN();
	HAL_SPI_Transmit(&hspi1, CMD, 1, 1000);
	FLASH_SPI_CS_UP();
}


/**
 * @brief     读取W25QXX的状态寄存器，W25Q64一共有2个状态寄存器
 * @param     reg  —— 状态寄存器编号(1~2)
 * @retval    状态寄存器的值
 */
static uint8_t W25QXX_ReadSR(uint8_t reg)
{
    uint8_t result = 0; 
    uint8_t send_buf[4] = {0x00,0x00,0x00,0x00};
    switch(reg)
    {
        case 1:
            send_buf[0] = READ_STATU_REGISTER_1;
        case 2:
            send_buf[0] = READ_STATU_REGISTER_2;
        case 0:
        default:
            send_buf[0] = READ_STATU_REGISTER_1;
    }
    
     /* 使能片选 */
		FLASH_SPI_CS_DOWN();
    if (HAL_OK == SPI1_TRANSMIT(send_buf, 4)) 
    {
        if (HAL_OK == SPI1_RECEIEVE(&result, 1)) 
        {
					FLASH_SPI_CS_UP();
          return result;
        }
    }
    /* 取消片选 */
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
    while((W25QXX_ReadSR(1) & 0x01) == 0x01); // 等待BUSY位清空
}

/**
 * @brief   读取SPI FLASH数据
 * @param   buffer      —— 数据存储区
 * @param   start_addr  —— 开始读取的地址(最大32bit)
 * @param   nbytes      —— 要读取的字节数(最大65535)
 * @retval  成功返回0，失败返回-1
 */
int W25QXX_Read(uint8_t* buffer, uint32_t start_addr, uint16_t nbytes)
{
	uint8_t cmd = READ_DATA_CMD;
	start_addr = start_addr << 8;
	W25QXX_Wait_Busy();
    
     /* 使能片选 */
	FLASH_SPI_CS_DOWN();

  SPI1_TRANSMIT(&cmd, 1);
	if (HAL_OK == SPI1_TRANSMIT((uint8_t*)&start_addr, 3)) 
	{
			if (HAL_OK == SPI1_RECEIEVE(buffer, nbytes)) 
			{
					FLASH_SPI_CS_UP();
					return 0;
			}
	}
	FLASH_SPI_CS_UP();
	return -1;
}

/**
 * @brief    页写入操作
 * @param    dat —— 要写入的数据缓冲区首地址
 * @param    WriteAddr —— 要写入的地址
 * @param   byte_to_write —— 要写入的字节数（0-256）
 * @retval    none
 */
void W25QXX_Page_Program(uint8_t* dat, uint32_t WriteAddr, uint16_t nbytes)
{
    uint8_t cmd = PAGE_PROGRAM_CMD;
    
    WriteAddr <<= 8;
    
    W25Q16_WRITE_ENABLE();
    
    /* 使能片选 */
    FLASH_SPI_CS_DOWN();
    
    SPI1_TRANSMIT(&cmd, 1);

    SPI1_TRANSMIT((uint8_t*)&WriteAddr, 3);
    
    SPI1_TRANSMIT(dat, nbytes);
    
		FLASH_SPI_CS_UP();
    
    W25QXX_Wait_Busy();
}

/**
 * @brief    W25QXX擦除一个扇区
 * @param   sector_addr    —— 扇区地址 根据实际容量设置
 * @retval  none
 * @note    阻塞操作
 */
void W25QXX_Erase_Sector(uint32_t sector_addr)
{
    uint8_t cmd = SECTOR_ERASE_CMD;
    
    sector_addr *= 4096;    //每个块有16个扇区，每个扇区的大小是4KB，需要换算为实际地址
    sector_addr <<= 8;
    
    W25Q16_WRITE_ENABLE();
    W25QXX_Wait_Busy();        //等待写使能完成
   
     /* 使能片选 */
    FLASH_SPI_CS_DOWN();
    
    SPI1_TRANSMIT(&cmd, 1);
    
    SPI1_TRANSMIT((uint8_t*)&sector_addr, 3);
    
    FLASH_SPI_CS_UP();
    
    W25QXX_Wait_Busy();       //等待扇区擦除完成
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	printf("Error");
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
