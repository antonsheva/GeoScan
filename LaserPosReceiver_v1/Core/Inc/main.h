/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "AN_spi.h"
#include "AN_define.h"
#include "AN_tdc.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
typedef struct{
  uint32_t pulseTm [TDC_FIFO_LEN];
  uint32_t rfClk   [TDC_FIFO_LEN];
  uint32_t stop    [TDC_FIFO_LEN];
  uint32_t tmLen   [TDC_FIFO_LEN];
  uint32_t origTime[TDC_FIFO_LEN];
  uint32_t beacons;
  uint32_t frstPls;
  uint32_t packPause;
  uint32_t prevPack;
  
}_TDC_DATA_STRUCT;

extern _TDC_DATA_STRUCT G_tdcData;
extern _TDC_DATA_STRUCT G_prevTdcData;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TDC_CS_Pin GPIO_PIN_4
#define TDC_CS_GPIO_Port GPIOC
#define TDC_INT_Pin GPIO_PIN_2
#define TDC_INT_GPIO_Port GPIOB
#define TDC_INT_EXTI_IRQn EXTI2_IRQn
#define TDC_RST_Pin GPIO_PIN_8
#define TDC_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#include "AN_spi.h"
#include "AN_define.h"
#include "AN_tdc.h"
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
