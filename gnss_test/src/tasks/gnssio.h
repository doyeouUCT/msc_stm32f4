/*
 * gnss.h
 *
 *  Created on: Mar 12, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_GNSSIO_H_
#define TASKS_GNSSIO_H_


#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "gnssinteger.h"

/* Receive buffer for DMA */
#define STM32_GNSS_USE_DMA
#define DMA_RX_BUFFER_SIZE          3000
#define GNSS_LOG_BUFFER_SIZE		3000

uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
uint8_t GNSS_LOG_Buffer[GNSS_LOG_BUFFER_SIZE];
uint16_t gnss_Length;

/* Results of GNSS Functions */
typedef enum {
	GNSS_OK = 0,		/* 0: Successful */
	GNSS_ERROR,			/* 1: R/W Error */
	GNSS_NOTRDY,		/* 2: Not Ready */
} GRESULT;

BYTE rsvr_usart( void );
GRESULT xmit_usart(BYTE);
void gnss_initialize (void);
void gnss_deinitialize (void);
void USART_GNSS_IRQHandler(void);
#ifdef STM32_GNSS_USE_DMA
void DMA_USART_RX_IRQHandler (void);
#endif

#endif /* TASKS_GNSSIO_H_ */
