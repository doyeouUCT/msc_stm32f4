/*
 * interrupt.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_INTERRUPT_H_
#define TASKS_INTERRUPT_H_

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

void exti_initialize (void);

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);

#endif /* TASKS_INTERRUPT_H_ */
