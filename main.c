/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "retarget.h"
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define PASS_UART 1
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void check_print(HAL_StatusTypeDef status, UART_HandleTypeDef *uarth, char recv[]);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void check_print(HAL_StatusTypeDef status, UART_HandleTypeDef *uarth, char recv[]){
	switch(status){
	case HAL_OK:
		if (PASS_UART){
			printf("Rx: %.*s \r\n", 1000-uarth->RxXferCount, recv);
		}
		break;
	case HAL_ERROR:
		printf("HAL_ERROR returned. Exiting.\r\n");
		exit(5);
	case HAL_BUSY:
		printf("HAL_BUSY returned. Exiting.\r\n");
		exit(5);
	case HAL_TIMEOUT:
		printf("HAL_TIMEOUT returned. Exiting.\r\n");
		exit(5);
	}
}

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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  RetargetInit(&huart2);
  /* USER CODE END 2 */

  printf("Hello, this program will create a Wi-Fi network from your ESP8086 board to test its functionalities.\r\n");

  /** PRELIM TEST **/
  /** AT_RST **/
  printf("\nFirst, let's try resetting the board.\r\n");

  char AT_CMD_RST[] = "AT+RST\r\n";
  char AT_received[1000];
  HAL_StatusTypeDef tx, rx;
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_RST, strlen(AT_CMD_RST), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);

  /** MAIN TEST **/
  printf("\nNow, I will create a Wi-Fi network with SSID \'ENGN500\' and password \'password\'.\r\n");

  char AT_CMD_MODE[] = "AT+CWMODE=3\r\n";
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_MODE, strlen(AT_CMD_MODE), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
  check_print(tx, &huart1, AT_received);

  char AT_SETUP_AP[] = "AT+CWSAP=\"ENGN500\",\"password\",1,3\r\n";
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_SETUP_AP, strlen(AT_SETUP_AP), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
  check_print(tx, &huart1, AT_received);

  char AT_LS_DEV[] = "AT+CWLIF\r\n";
  printf("Now waiting for incoming connection.\r\n");
  bool connected = false;
  while (!connected){
	  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_LS_DEV, strlen(AT_LS_DEV), 2000);
	  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
	  if (huart1.RxXferCount < 983){ // longer than input cmd - a.k.a. ip addr returned
		  connected = true;
		  printf("Connection made from: ");
		  uint8_t cur = 11;
		  while(AT_received[cur] != 44 && cur < 1000-huart1.RxXferCount){
			  printf("%c", AT_received[cur]);
			  cur += 1;
		  }
		  printf("\r\n");
	  }
  }

  char AT_IF[] = "AT+CIFSR\r\n";
  printf("IP address of ESP Chip: ");
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_IF, strlen(AT_IF), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
  int cur = 24;
  while(AT_received[cur] != 44 && cur < 1000-huart1.RxXferCount){
	  printf("%c", AT_received[cur]);
	  cur += 1;
  }
  printf("\r\n");

  char AT_CIPMUX[] = "AT+CIPMUX=1\r\n";
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CIPMUX, strlen(AT_CIPMUX), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
  check_print(tx, &huart1, AT_received);

  char AT_CIP_SV[] = "AT+CIPSERVER=1,80\r\n";
  printf("Now opening HTTP (80) Socket\r\n");
  tx = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CIP_SV, strlen(AT_CIP_SV), 2000);
  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
  check_print(tx, &huart1, AT_received);

  char AT_IPD_READ[] = "+IPD";
  while (1){
	  rx = HAL_UART_Receive(&huart1, (uint8_t*)AT_received, 1000, 4000);
	  check_print(tx, &huart1, AT_received);
  }


//  char AT_CMD_AT1[] = "AT+CWMODE=?\r\n";
//  retval = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_AT1, strlen(AT_CMD_AT1), 2000);
//  retval = HAL_UART_Receive(&huart1, (uint8_t*)ATreceived, 1000, 4000);
//  printf("Rx: %.*s \r\n",1000-huart1.RxXferCount, ATreceived);
//
//  char AT_CMD_AT2[] = "AT+CWLAP\r\n";
//  retval = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_AT2, strlen(AT_CMD_AT2), 2000);
//  retval = HAL_UART_Receive(&huart1, (uint8_t*)ATreceived, 1000, 4000);
//  printf("Rx: %.*s \r\n",1000-huart1.RxXferCount, ATreceived);
//
//  char AT_CMD_AT3[] = "AT+CWJAP_CUR=\"A8004T\",\"msc634800\"\r\n";
//  retval = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_AT3, strlen(AT_CMD_AT3), 2000);
//  retval = HAL_UART_Receive(&huart1, (uint8_t*)ATreceived, 1000, 4000);
//  printf("Rx: %.*s \r\n",1000-huart1.RxXferCount, ATreceived);
//
//  char AT_CMD_AT4[] = "AT+CWJAP_CUR?\r\n";
//  retval = HAL_UART_Transmit(&huart1, (uint8_t*)AT_CMD_AT4, strlen(AT_CMD_AT4), 2000);
//  retval = HAL_UART_Receive(&huart1, (uint8_t*)ATreceived, 1000, 4000);
//  printf("Rx: %.*s \r\n",1000-huart1.RxXferCount, ATreceived);

  printf("TEST COMPLETE.\r\n");

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */


    /* USER COsDE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
