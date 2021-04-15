/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define THROTTLE_STEPS_FULL_RANGE (3300*THROTTLE_STEPS/(THROTTLE_MAX_mV-THROTTLE_MIN_mV))
#define THROTTLE_STEPS 1024
#define THROTTLE_OFF_mV 1000
#define THROTTLE_MIN_mV 1600
#define THROTTLE_MAX_mV 3300

#define CS_I2C_SPI_Pin GPIO_PIN_3
#define CS_I2C_SPI_GPIO_Port GPIOE
#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define PH0_OSC_IN_Pin GPIO_PIN_0
#define PH0_OSC_IN_GPIO_Port GPIOH
#define PH1_OSC_OUT_Pin GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_Port GPIOH
#define OTG_FS_PowerSwitchOn_Pin GPIO_PIN_0
#define OTG_FS_PowerSwitchOn_GPIO_Port GPIOC
#define PDM_OUT_Pin GPIO_PIN_3
#define PDM_OUT_GPIO_Port GPIOC
#define B1_Pin GPIO_PIN_0
#define B1_GPIO_Port GPIOA
#define HALL1A_Pin GPIO_PIN_1
#define HALL1A_GPIO_Port GPIOA
#define HALL1B_Pin GPIO_PIN_2
#define HALL1B_GPIO_Port GPIOA
#define HALL1C_Pin GPIO_PIN_5
#define HALL1C_GPIO_Port GPIOA
#define HALL2A_Pin GPIO_PIN_6
#define HALL2A_GPIO_Port GPIOA
#define HALL2B_Pin GPIO_PIN_7
#define HALL2B_GPIO_Port GPIOA
#define HALL2C_Pin GPIO_PIN_0
#define HALL2C_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define THROTTLE1_Pin GPIO_PIN_9
#define THROTTLE1_GPIO_Port GPIOE
#define THROTTLE2_Pin GPIO_PIN_11
#define THROTTLE2_GPIO_Port GPIOE
#define THROTTLE3_Pin GPIO_PIN_13
#define THROTTLE3_GPIO_Port GPIOE
#define BRAKEX_Pin GPIO_PIN_14
#define BRAKEX_GPIO_Port GPIOE
#define CLK_IN_Pin GPIO_PIN_10
#define CLK_IN_GPIO_Port GPIOB
#define BRAKE1_Pin GPIO_PIN_14
#define BRAKE1_GPIO_Port GPIOB
#define DIRECTION1_Pin GPIO_PIN_15
#define DIRECTION1_GPIO_Port GPIOB
#define BRAKE2_Pin GPIO_PIN_8
#define BRAKE2_GPIO_Port GPIOD
#define DIRECTION2_Pin GPIO_PIN_9
#define DIRECTION2_GPIO_Port GPIOD
#define BRAKE3_Pin GPIO_PIN_10
#define BRAKE3_GPIO_Port GPIOD
#define DIRECTION3_Pin GPIO_PIN_11
#define DIRECTION3_GPIO_Port GPIOD
#define LD4_Pin GPIO_PIN_12
#define LD4_GPIO_Port GPIOD
#define LD3_Pin GPIO_PIN_13
#define LD3_GPIO_Port GPIOD
#define LD5_Pin GPIO_PIN_14
#define LD5_GPIO_Port GPIOD
#define LD6_Pin GPIO_PIN_15
#define LD6_GPIO_Port GPIOD
#define IS_LCU_b_Pin GPIO_PIN_6
#define IS_LCU_b_GPIO_Port GPIOC
#define IS_RCU_b_Pin GPIO_PIN_7
#define IS_RCU_b_GPIO_Port GPIOC
#define VBUS_FS_Pin GPIO_PIN_9
#define VBUS_FS_GPIO_Port GPIOA
#define OTG_FS_ID_Pin GPIO_PIN_10
#define OTG_FS_ID_GPIO_Port GPIOA
#define OTG_FS_DM_Pin GPIO_PIN_11
#define OTG_FS_DM_GPIO_Port GPIOA
#define OTG_FS_DP_Pin GPIO_PIN_12
#define OTG_FS_DP_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define UART5_GPIO_Pin GPIO_PIN_0
#define UART5_GPIO_GPIO_Port GPIOD
#define Audio_RST_Pin GPIO_PIN_4
#define Audio_RST_GPIO_Port GPIOD
#define OTG_FS_OverCurrent_Pin GPIO_PIN_5
#define OTG_FS_OverCurrent_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define HALL3A_Pin GPIO_PIN_4
#define HALL3A_GPIO_Port GPIOB
#define HALL3C_Pin GPIO_PIN_8
#define HALL3C_GPIO_Port GPIOB
#define HALL3B_Pin GPIO_PIN_9
#define HALL3B_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define ENABLE_SERVOS 0
#define ENABLE_MOTOR1 1
#define ENABLE_MOTOR2 1
#define ENABLE_MOTOR3 1
#define SKID_STEERING (!ENABLE_SERVOS)
#define HARD_SPEED_LIMIT 1.5f
#define HARD_CURVATURE_LIMIT 10.0f
#define FLIP_HALLS_MOTOR1 0
#define FLIP_HALLS_MOTOR2 0
#define FLIP_HALLS_MOTOR3 0
#define USE_ASCII_INSTEAD_OF_PROTOS 1
#define DO_INTERPOLATION_OF_TRAJECTORIES 1
#define DEBUG_SEND_FULL_COMMANDS_IN_RESPONSES 1
#define DEBUG_TIME_IS_ASSUMED_TO_START_WITH_FIRST_TRAJECTORY 1
#define DEBUG_DONT_SEND_COMPLETIONS 1
#define M_PIf 3.14159265358979f
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
