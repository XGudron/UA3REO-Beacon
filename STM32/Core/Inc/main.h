/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define float64_t double
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern IWDG_HandleTypeDef hiwdg;
extern SPI_HandleTypeDef hspi1;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define ONLY_DASHES_Pin GPIO_PIN_0
#define ONLY_DASHES_GPIO_Port GPIOA
#define BAND_A2_Pin GPIO_PIN_2
#define BAND_A2_GPIO_Port GPIOA
#define MAX_CE_Pin GPIO_PIN_3
#define MAX_CE_GPIO_Port GPIOA
#define MAX_CS_Pin GPIO_PIN_4
#define MAX_CS_GPIO_Port GPIOA
#define MAX_LD_Pin GPIO_PIN_6
#define MAX_LD_GPIO_Port GPIOA
#define BAND_B0_Pin GPIO_PIN_0
#define BAND_B0_GPIO_Port GPIOB
#define BAND_B1_Pin GPIO_PIN_1
#define BAND_B1_GPIO_Port GPIOB
#define BAND_B10_Pin GPIO_PIN_10
#define BAND_B10_GPIO_Port GPIOB
#define BAND_B11_Pin GPIO_PIN_11
#define BAND_B11_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
