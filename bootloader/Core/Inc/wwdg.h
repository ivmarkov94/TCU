/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    wwdg.h
  * @brief   This file contains all the function prototypes for
  *          the wwdg.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#ifndef __WWDG_H__
#define __WWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_WWDG_Init(void);

/* USER CODE BEGIN Prototypes */
void wwdg_refresh(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __WWDG_H__ */

