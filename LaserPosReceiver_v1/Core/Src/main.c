/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
 
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef signed char        int8_t;
typedef short              int16_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DAC_HandleTypeDef hdac;

SPI_HandleTypeDef hspi1;

// UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_tx;
DMA_HandleTypeDef hdma_usart6_rx;

/* USER CODE BEGIN PV */
//TIM_HandleTypeDef htim2;
char str_tx[128];

_TDC_DATA_STRUCT G_tdcData;
_TDC_DATA_STRUCT G_prevTdcData;
uint8_t  m_beacons    [TDC_FIFO_LEN]            = {0};
uint32_t m_fifoTimes  [TDC_FIFO_LEN][FIFO_LEN]  = {0};
uint32_t m_result     [TDC_FIFO_LEN];                
uint32_t m_fifoPause  [FIFO_LEN]                = {0};
uint32_t m_pauseBetweenPacks = 0;
uint16_t m_tdcWatchdogTm = 0;
uint8_t  m_beaconsQty;
uint8_t  m_seconds = 0;
uint16_t m_packFrq = 0;
uint16_t m_packCnt = 0;
uint8_t m_reportFrqDivider = 30; //Делитель таймера выдачи отчетов 

uint8_t m_uartRxBuff[16];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_DAC_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
 
int  ACheckData(int bcnQty);
void ASearchMissingOneBeacon();
void ASearchMissingTwoBeacon();
void ASearchMissingThreeBeacon();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void AShiftFifo(){
  uint32_t prevVal[PULSE_QTY-1]; 
  static uint64_t summVal[PULSE_QTY-1] = {0}; 
  static uint64_t summDelta = 0;
  static uint32_t prevDelta = 0;
  
  for(int k=0; k<PULSE_QTY-1; k++){
    prevVal[k] = m_fifoTimes[k][FIFO_LEN-1];
    for(int i=FIFO_LEN-1; i>0; i--){
      m_fifoTimes[k][i] = m_fifoTimes[k][i-1];

    }
    m_fifoTimes[k][0] = (TM[k]>0) ? TM[k] : m_fifoTimes[k][1];
    summVal[k] += m_fifoTimes[k][0];
    summVal[k] -= prevVal[k];
    m_result[k] = summVal[k]/FIFO_LEN;
  }

  prevDelta = m_fifoPause[FIFO_LEN-1];
  for(int i=FIFO_LEN-1; i>0; i--){
    m_fifoPause[i] = m_fifoPause[i-1];
  }
  m_fifoPause[0] = G_tdcData.packPause;
  summDelta += m_fifoPause[0];
  summDelta -= prevDelta;
  m_pauseBetweenPacks = summDelta/FIFO_LEN; 
}

void ACalcPackFrqDivider(){
  G_uartPackDiv = m_packFrq/G_uartPackFrq;
}
 
void APrintTdcData(){
    PRT.bcsQty = m_beaconsQty;

    for(int i=0; i<PULSE_QTY; i++){
      PRT.bcs[i] = m_beacons[i];
      PRT.tm [i] = m_result[i];
    }
  
    APrintData(PRINT_PARAM_TDC, 0); 
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART6){
    AParseUartData(m_uartRxBuff);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){  
  static int cnt = 0;
  static int missBcn = 0;
  
  m_tdcWatchdogTm = 0; 
  m_packCnt++;
  m_beaconsQty = ATdcRead();
 
  if(m_beaconsQty < PULSE_QTY){
    ACheckData(m_beaconsQty);
  }else{
    missBcn = 0;
    for(int i=0; i<PULSE_QTY; i++)m_beacons[i] = 1;
  } 
  AShiftFifo();


  
  if(!(++cnt%G_uartPackDiv))APrintTdcData(); 

}

#define S_P_D 15 //shif prev data 
#define D_TM 1000 //delta tm

int ACheckBeacon1(){
  
  if((TM[0]> (m_fifoTimes[1][S_P_D]-D_TM)) && (TM[0] < (m_fifoTimes[1][S_P_D] + D_TM))){
        m_beacons[1] = 1;
        m_beacons[2] = 1;
        m_beacons[3] = 1;    

        TM[2] = TM[1];
        TM[1] = TM[0];
        TM[0] = FT[0][1];
        return 1;    
    }
  return 0; 
}

int ACheckBeacon2(){
  if((TM[0]> ((FT[0][S_P_D]+FT[1][S_P_D])-D_TM)) && (TM[0] < (FT[0][S_P_D]+FT[1][S_P_D]) + D_TM)){
        m_beacons[0] = 1;
        m_beacons[2] = 1;
        m_beacons[3] = 1;    
        
        TM[2] = TM[1];
 
        TM[0] = FT[0][1];
        TM[1] = FT[1][1];

        return 1;    
    }
  return 0;  
}

int ACheckBeacon3(){
  if((TM[1]> ((FT[1][S_P_D]+FT[2][S_P_D])-D_TM)) && (TM[1] < (FT[1][S_P_D]+FT[2][S_P_D]) + D_TM)){
        m_beacons[0] = 1;
        m_beacons[1] = 1;
        m_beacons[3] = 1; 
        
        TM[2] = FT[2][1];
        TM[1] = FT[1][1];  
        return 1;    
    }
  return 0; 
}

int ACheckBeacon4(){
  if((TM[1]> ((FT[1][S_P_D])-D_TM)) && (TM[1] < (FT[1][S_P_D]) + D_TM)){
        m_beacons[0] = 1;
        m_beacons[1] = 1;
        m_beacons[2] = 1; 
        
        TM[2] = FT[2][1];
        return 1;    
    }
  return 0;  
}

void ASearchMissingOneBeacon(){
  if(ACheckBeacon3())return;    
  if(ACheckBeacon1())return;
  if(ACheckBeacon2())return;

  if(ACheckBeacon4())return;     
}

void ASearchMissingTwoBeacon(){

}

void ASearchMissingThreeBeacon(){

}

int ACheckData(int bcnQty){
  for(int i=0; i<PULSE_QTY; i++)m_beacons[i] = 0;  
  ASearchMissingOneBeacon();
  return 0;
}

void HAL_IncTick(void)
{
  static uint16_t cnt = 0;

  cnt++;
  if(cnt == 1000){
    m_packFrq = m_packCnt;
    m_packCnt = 0;
    m_seconds++;
    cnt = 0;
    if((G_uartPackFrq > 0)&&(m_packFrq > G_uartPackFrq)){
      G_uartPackDiv = m_packFrq/G_uartPackFrq;
    }
  }
  if(m_tdcWatchdogTm++ > 100){
    __disable_irq();
     ATdcInit();
    __enable_irq();
     ATdcRead();
    m_tdcWatchdogTm = 0;
  }
  uwTick += uwTickFreq;
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  uint8_t q = 0;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  MX_DAC_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  ATdcInit();
  for(int i=0; i<4; i++)ATdcRead(&q);
  HAL_GPIO_WritePin(TDC_RST_GPIO_Port, TDC_RST_Pin, 1);

  HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
  HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R, G_dacVal);

  HAL_UART_Receive_DMA(&huart6, m_uartRxBuff, UART_BUFF_SIZE);

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
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TDC_CS_GPIO_Port, TDC_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TDC_RST_GPIO_Port, TDC_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TDC_CS_Pin */
  GPIO_InitStruct.Pin = TDC_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TDC_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TDC_INT_Pin */
  GPIO_InitStruct.Pin = TDC_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TDC_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TDC_RST_Pin */
  GPIO_InitStruct.Pin = TDC_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TDC_RST_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//static void MX_TIM2_Init(void)
//{
//
//  /* USER CODE BEGIN TIM2_Init 0 */
//
//  /* USER CODE END TIM2_Init 0 */
//
//  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
//  TIM_MasterConfigTypeDef sMasterConfig = {0};
//
//  /* USER CODE BEGIN TIM2_Init 1 */
//
//  /* USER CODE END TIM2_Init 1 */
//  htim2.Instance = TIM2;
//  htim2.Init.Prescaler = 21600;
//  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
//  htim2.Init.Period = 100;
//  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
//  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN TIM2_Init 2 */
//
//  /* USER CODE END TIM2_Init 2 */
//
//}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
