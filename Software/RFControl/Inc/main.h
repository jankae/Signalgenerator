/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DET_EN_Pin GPIO_PIN_13
#define DET_EN_GPIO_Port GPIOC
#define DET_SCI4_Pin GPIO_PIN_14
#define DET_SCI4_GPIO_Port GPIOC
#define DB15_3_Pin GPIO_PIN_15
#define DB15_3_GPIO_Port GPIOC
#define DB15_2_Pin GPIO_PIN_0
#define DB15_2_GPIO_Port GPIOF
#define DB15_1_Pin GPIO_PIN_1
#define DB15_1_GPIO_Port GPIOF
#define EN_DIRECT_Pin GPIO_PIN_2
#define EN_DIRECT_GPIO_Port GPIOA
#define EN_INTREF_Pin GPIO_PIN_3
#define EN_INTREF_GPIO_Port GPIOA
#define ADF_MUX_Pin GPIO_PIN_4
#define ADF_MUX_GPIO_Port GPIOA
#define ADF_CE_Pin GPIO_PIN_5
#define ADF_CE_GPIO_Port GPIOA
#define FPGA_CS_Pin GPIO_PIN_6
#define FPGA_CS_GPIO_Port GPIOA
#define MIX_EN_Pin GPIO_PIN_0
#define MIX_EN_GPIO_Port GPIOB
#define OFFSET_CS_Pin GPIO_PIN_1
#define OFFSET_CS_GPIO_Port GPIOB
#define SYNTH_RF_EN_Pin GPIO_PIN_2
#define SYNTH_RF_EN_GPIO_Port GPIOB
#define SYNTH_MUX_Pin GPIO_PIN_10
#define SYNTH_MUX_GPIO_Port GPIOB
#define SYNTH_LE_Pin GPIO_PIN_11
#define SYNTH_LE_GPIO_Port GPIOB
#define SYNTH_CE_Pin GPIO_PIN_11
#define SYNTH_CE_GPIO_Port GPIOA
#define SYNTH_LD_Pin GPIO_PIN_12
#define SYNTH_LD_GPIO_Port GPIOA
#define ADF_LE_Pin GPIO_PIN_15
#define ADF_LE_GPIO_Port GPIOA
#define DBM_CS_Pin GPIO_PIN_6
#define DBM_CS_GPIO_Port GPIOB
#define DET_SCI3_Pin GPIO_PIN_7
#define DET_SCI3_GPIO_Port GPIOB
#define DET_SCI1_Pin GPIO_PIN_8
#define DET_SCI1_GPIO_Port GPIOB
#define DET_SCI2_Pin GPIO_PIN_9
#define DET_SCI2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
