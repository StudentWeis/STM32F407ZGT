/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ov2640.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#define RES160X120
// #define RES320X240
// #define RES640X480
// #define RES800x600
// #define RES1024x768
// #define RES1280x960
#ifdef RES320X240
enum imageResolution imgRes = RES_320X240;
uint8_t frameBuffer[RES_320X240] = {0};
#endif

#ifdef RES160X120
enum imageResolution imgRes = RES_160X120;
uint8_t frameBuffer[RES_160X120] = {0};
#endif

uint8_t mutex = 0;
uint16_t bufferPointer = 0;
uint8_t headerFound = 0;

uint8_t Tbuffer1[2] = {0xff, 0x01};
uint8_t Tbuffer2[2] = {0x12, 0x80};
uint8_t Tbuffer3[1] = {0x1c};
uint8_t RBuffer[1] = {0};

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
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_USART3_UART_Init();
  MX_I2C2_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  // 等待�???�???
  uint8_t a[1];
  printf("Input a.\r\n");
  scanf("%c", a);
  if (a[0] == 'a')
    printf("Hello World!\r\n");

  // HAL_GPIO_WritePin(CAMERA_PWDN_GPIO_Port, CAMERA_PWDN_Pin, GPIO_PIN_RESET);
  // HAL_Delay(10);
  // HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_RESET);
  // HAL_Delay(10);
  // HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_SET);
  // HAL_Delay(10);

  // HAL_I2C_Master_Transmit(&hi2c2, 0xc0, Tbuffer1, 2, 100);
  // //SCCB_Write(0xff, 0x01);
  // HAL_Delay(10);

  // HAL_I2C_Master_Transmit(&hi2c2, 0xc0, Tbuffer2, 2, 100);
  // HAL_Delay(10);

  // OV2640_ResolutionOptions(imgRes);

  OV2640_Init(&hi2c2, &hdcmi);
  HAL_Delay(10);
  OV2640_ResolutionOptions(imgRes);
  HAL_Delay(10);

  HAL_I2C_Mem_Read(&hi2c2, 0x60, 0x0A, 1, RBuffer, 1, 100);
  printf("%x\r\n", RBuffer[0]);

  //  // 等待下一环节
  //	a[0] = 0;
  //  printf("Input b.\r\n");
  //  scanf("%c", a);
  //  if (a[0] == 'b')
  // OV2640_Init(&hi2c2, &hdcmi);
  // HAL_Delay(10);
  // OV2640_ResolutionOptions(imgRes);
  // HAL_Delay(10);
  mutex = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (mutex == 1)
    {
      __HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_FRAME);
      memset(frameBuffer, 0, sizeof frameBuffer);
      OV2640_CaptureSnapshot((uint32_t)frameBuffer, imgRes);

      while (1)
      {
        if (headerFound == 0 && frameBuffer[bufferPointer] == 0xFF && frameBuffer[bufferPointer + 1] == 0xD8)
        {
          headerFound = 1;
#ifdef DEBUG
          printf("Found header of JPEG file \r\n");
#endif
        }
        if (headerFound == 1 && frameBuffer[bufferPointer] == 0xFF && frameBuffer[bufferPointer + 1] == 0xD9)
        {
          bufferPointer = bufferPointer + 2;
#ifdef DEBUG
          printf("Found EOF of JPEG file \r\n");
#endif
          headerFound = 0;
          break;
        }

        if (bufferPointer >= 65535)
        {
          //break;
					bufferPointer = 0;
					printf("Flag");
        }
        bufferPointer++;
      }
#ifdef DEBUG
      printf("Image size: %d bytes \r\n", bufferPointer);
#endif

      HAL_UART_Transmit_DMA(&huart3, frameBuffer, bufferPointer); // Use of DMA may be necessary for larger data streams.
      bufferPointer = 0;
      mutex = 0;
    }
    else
    {
      mutex = 1;
    }
    while (1)
    {
      /* code */
    }
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
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}

int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart3, &ch, 1, 0xffff);
  return ch;
}

void HAL_Delay_US(uint32_t Delay_us)
{
  __HAL_TIM_SetCounter(&htim7, 0);
  __HAL_TIM_ENABLE(&htim7);
  while (__HAL_TIM_GetCounter(&htim7) < Delay_us)
    ;
  __HAL_TIM_DISABLE(&htim7);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
