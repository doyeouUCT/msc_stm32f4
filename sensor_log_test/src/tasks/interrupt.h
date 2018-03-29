/*
 * interrupt.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_INTERRUPT_H_
#define TASKS_INTERRUPT_H_

/* Standard includes. */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/* Task includes. */
#include "tasks/global.h"
#include "tasks/sensorio.h"
#include "tasks/timeio.h"

uint8_t timer_enabled;

/* Global function prototypes */
void exti_initialize (void);
void EXTILine_ACCEL_IRQHandler(void);
void EXTILine_GYRO_IRQHandler(void);
void EXTILine_MAG_IRQHandler(void);
void EXTILine_BARO_IRQHandler(void);

/* EXTI connection pin defined. */
#define RCC_AHBPeriphClockCmd_EXTI_GPIO			RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_EXTI_GPIO		   			RCC_AHB1Periph_GPIOA
#define GPIO_EXTI		              			GPIOA
#define GPIO_Pin_EXTI_ACCEL	           			GPIO_Pin_0
#define GPIO_Pin_EXTI_GYRO             			GPIO_Pin_1
#define GPIO_Pin_EXTI_MAG	           			GPIO_Pin_2
#define GPIO_Pin_EXTI_BARO             			GPIO_Pin_3
#define GPIO_Pin_EXTI_PPS						GPIO_Pin_4

#define RCC_APBPeriphClockCmd_EXTI				RCC_APB2PeriphClockCmd
#define RCC_APBPeriph_EXTI						RCC_APB2Periph_SYSCFG

#define GPIO_EXTILine_Config					SYSCFG_EXTILineConfig
#define GPIO_Pin_EXTI_Src						EXTI_PortSourceGPIOA
#define GPIO_Pin_EXTI_ACCEL_Src					EXTI_PinSource0
#define GPIO_Pin_EXTI_GYRO_Src					EXTI_PinSource1
#define GPIO_Pin_EXTI_MAG_Src					EXTI_PinSource2
#define GPIO_Pin_EXTI_BARO_Src					EXTI_PinSource3
#define GPIO_Pin_EXTI_PPS_Src					EXTI_PinSource4

#define EXTILine_ACCEL							EXTI_Line0
#define EXTILine_GYRO							EXTI_Line1
#define EXTILine_MAG							EXTI_Line2
#define EXTILine_BARO							EXTI_Line3
#define EXTILine_PPS							EXTI_Line4

#define EXTILine_ACCEL_IRQn						EXTI0_IRQn
#define EXTILine_GYRO_IRQn						EXTI1_IRQn
#define EXTILine_MAG_IRQn						EXTI2_IRQn
#define EXTILine_BARO_IRQn						EXTI3_IRQn
#define EXTILine_PPS_IRQn						EXTI4_IRQn

#define EXTILine_Priority						0x00
#define EXTILine_ACCEL_Priority					0x00
#define EXTILine_GYRO_Priority					0x01
#define EXTILine_MAG_Priority					0x02
#define EXTILine_BARO_Priority					0x03
#define EXTILine_PPS_Priority					0x04

#define EXTILine_ACCEL_IRQHandler				EXTI0_IRQHandler
#define EXTILine_GYRO_IRQHandler				EXTI1_IRQHandler
#define EXTILine_MAG_IRQHandler					EXTI2_IRQHandler
#define EXTILine_BARO_IRQHandler				EXTI3_IRQHandler
#define EXTILine_PPS_IRQHandler					EXTI4_IRQHandler

#endif /* TASKS_INTERRUPT_H_ */
