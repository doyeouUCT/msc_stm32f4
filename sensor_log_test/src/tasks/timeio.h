/*
 * timeio.h
 *
 *  Created on: Mar 20, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_TIMEIO_H_
#define TASKS_TIMEIO_H_

/* Standard includes. */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/* Time synchronization variable */
typedef struct {
	volatile uint32_t counterI;		/* PPS update : 1 Hz   */
	volatile uint16_t counterII;	/* ARR update : 500 Hz */
	volatile uint16_t counterIII;	/* CNR update : 1 MHz  */
}TIME;

TIME time_stamp;

void timestamp_initialize (void);
void timestamp_enable (void);
void TIMER_PPS_IRQHandler (void);

/* Timer PPS connection pin defined. */
#define RCC_APBPeriphClockCmd_PPS				RCC_APB1PeriphClockCmd
#define RCC_APBPeriph_PPS						RCC_APB1Periph_TIM3

#define TIMER_PPS								TIM3
#define TIMER_PPS_PSC							83
#define TIMER_PPS_ARR							1999

#define TIMER_PPS_IRQn							TIM3_IRQn
#define TIMER_PPS_IRQHandler					TIM3_IRQHandler
#define TIMER_PPS_Priority						0x00


#endif /* TASKS_TIMEIO_H_ */
