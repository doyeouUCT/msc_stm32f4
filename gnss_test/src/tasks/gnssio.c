/*
 * gnssio.c
 *
 *  Created on: Mar 12, 2018
 *      Author: Do Yeou Ku
 */

#include "gnssio.h"

/* USART connection pin defined. */
#define RCC_AHBPeriphClockCmd_USART_GNSS		RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_USART_GNSS	   			RCC_AHB1Periph_GPIOB
#define RCC_APBPeriphClockCmd_USART_GNSS		RCC_APB2PeriphClockCmd
#define RCC_APBPeriph_USART_GNSS	   			RCC_APB2Periph_USART1
#define USART_GNSS								USART1
#define GPIO_USART_GNSS							GPIOB
#define GPIO_Pin_USART_GNSS_TX					GPIO_Pin_6
#define GPIO_Pin_USART_GNSS_RX					GPIO_Pin_7

#define GPIO_Pin_USART_GNSS_TX_Src				GPIO_PinSource6
#define GPIO_Pin_USART_GNSS_RX_Src				GPIO_PinSource7

#define GPIO_AF_USART_GNSS						GPIO_AF_USART1

#define USART_BaudRate_USART_GNSS				115200
#define USART_GNSS_IT							USART_IT_IDLE
#define USART_GNSS_FLAG							USART_FLAG_IDLE
#define USART_GNSS_IRQn							USART1_IRQn
#define USART_GNSS_IRQHandler					USART1_IRQHandler

/* DMA connection defined. */
#define RCC_AHBPeriphClockCmd_USART_DMA			RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_DMA						RCC_AHB1Periph_DMA2
#define DMA_Channel_USART_GNSS_RX				DMA_Channel_4
#define DMA_Channel_USART_GNSS_TX				DMA_Channel_4
#define DMA_Stream_USART_GNSS_RX				DMA2_Stream2
#define DMA_Stream_USART_GNSS_TX				DMA2_Stream7

#define DMA_USART_IT							DMA_IT_TC
#define DMA_Stream_USART_IT						DMA_IT_TCIF2
#define DMA_FLAG_USART_GNSS_TC_RX				DMA_FLAG_TCIF2
#define DMA_FLAG_USART_GNSS_TC_TX				DMA_FLAG_TCIF7

#define DMA_USART_RX_IRQn						DMA2_Stream2_IRQn
#define DMA_USART_TX_IRQn						DMA2_Stream7_IRQn
#define DMA_USART_RX_IRQHandler					DMA2_Stream2_IRQHandler
#define DMA_USART_TX_IRQHandler					DMA2_Stream7_IRQHandler


/*-----------------------------------------------------------------------
 Module Private Functions and Variables
 ------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Transmit/Receive a byte via USART  (Platform dependent) */
/*-----------------------------------------------------------------------*/

GRESULT xmit_usart( BYTE out )
{
	USART_SendData(USART_GNSS,(uint16_t)out);
	while(USART_GetFlagStatus(USART_GNSS, USART_FLAG_TXE) != SET){ ; }
	return GNSS_OK;
}

/*-----------------------------------------------------------------------*/
/* Receive a byte via USART  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

BYTE rsvr_usart( void )
{
	while(USART_GetFlagStatus(USART_GNSS, USART_FLAG_RXNE) != SET){ ; }
	uint8_t tmp = (uint8_t)USART_ReceiveData(USART_GNSS);
	return tmp;
}

/* Alternative macro to receive data fast */
#define rsvr_usart_m(dst)  *(dst)=rsvr_usart()

/*-----------------------------------------------------------------------*/
/* Initialize the BUFFER ARRAY						                     */
/*-----------------------------------------------------------------------*/

static
void zero_dma_gnss_memory (void)
{
	for (int i = 0; i < DMA_RX_BUFFER_SIZE; i++){
		DMA_RX_Buffer[i] = 0;
	}
}

static
void zero_gnss_memory (void)
{
	for (int i = 0; i < GNSS_LOG_BUFFER_SIZE; i++){
		GNSS_LOG_Buffer[i] = 0;
	}
}
/*-----------------------------------------------------------------------*/
/* Initialize the GNSS - USART bus					                     */
/*-----------------------------------------------------------------------*/

static
void gnss_on (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable GPIO clock for USART pins */
	RCC_AHBPeriphClockCmd_USART_GNSS(RCC_AHBPeriph_USART_GNSS, ENABLE);
	/* Enable USART clock */
	RCC_APBPeriphClockCmd_USART_GNSS(RCC_APBPeriph_USART_GNSS, ENABLE);

	/* USART GPIO Configuration ----------------------------------------*/

	/* Configure USART pins: TX & RX */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	/* USART TX pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_USART_GNSS_TX;
	GPIO_Init(GPIO_USART_GNSS, &GPIO_InitStructure);

	/* USART RX pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_USART_GNSS_RX;
	GPIO_Init(GPIO_USART_GNSS, &GPIO_InitStructure);

	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIO_USART_GNSS, GPIO_Pin_USART_GNSS_TX_Src, GPIO_AF_USART_GNSS); //
	GPIO_PinAFConfig(GPIO_USART_GNSS, GPIO_Pin_USART_GNSS_RX_Src, GPIO_AF_USART_GNSS);

	/* USART configuration */
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_BaudRate = USART_BaudRate_USART_GNSS;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART_GNSS, &USART_InitStructure);

	/* USART IDLE interrupt configuration */
	NVIC_InitStructure.NVIC_IRQChannel = USART_GNSS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART_GNSS, USART_GNSS_IT, ENABLE);

	USART_Cmd(USART_GNSS, ENABLE);

#ifdef STM32_GNSS_USE_DMA
	USART_DMACmd(USART_GNSS, USART_DMAReq_Rx, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd_USART_DMA(RCC_AHBPeriph_DMA,ENABLE);

	/* De-initialize DMA RX & TX Stream */
	DMA_DeInit(DMA_Stream_USART_GNSS_RX);
	while (DMA_GetCmdStatus(DMA_Stream_USART_GNSS_RX ) != DISABLE) { ; }
	DMA_DeInit(DMA_Stream_USART_GNSS_TX);
	while (DMA_GetCmdStatus(DMA_Stream_USART_GNSS_TX ) != DISABLE) { ; }

	/* shared DMA configuration values */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART_GNSS->DR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

	DMA_InitStructure.DMA_Channel = DMA_Channel_USART_GNSS_RX;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)DMA_RX_Buffer;
	DMA_InitStructure.DMA_BufferSize = DMA_RX_BUFFER_SIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init(DMA_Stream_USART_GNSS_RX, &DMA_InitStructure);

	// enable the interrupt in the NVIC
	NVIC_InitStructure.NVIC_IRQChannel = DMA_USART_RX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA_Stream_USART_GNSS_RX, DMA_USART_IT, ENABLE);

	DMA_Cmd(DMA_Stream_USART_GNSS_RX, ENABLE);
	while (DMA_GetCmdStatus(DMA_Stream_USART_GNSS_RX ) != ENABLE) { ; }

#endif
}

/*-----------------------------------------------------------------------*/
/* De-initialize the GNSS - USART bus				                     */
/*-----------------------------------------------------------------------*/

static
void gnss_off(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	USART_DeInit(USART_GNSS);
	USART_Cmd(USART_GNSS, DISABLE);
	RCC_APBPeriphClockCmd_USART_GNSS(RCC_APBPeriph_USART_GNSS, DISABLE);

	/* All SPI-Pins to input with weak internal pull-downs */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_USART_GNSS_RX|GPIO_Pin_USART_GNSS_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIO_USART_GNSS, &GPIO_InitStructure);
}

/*-----------------------------------------------------------------------

 Public Functions

 ------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize GNSS                                               */
/*-----------------------------------------------------------------------*/

void gnss_initialize (void)
{
	zero_gnss_memory ();
	zero_gnss_memory ();
	gnss_on();
}

/*-----------------------------------------------------------------------*/
/* Initialize GNSS                                               */
/*-----------------------------------------------------------------------*/

void gnss_deinitialize (void)
{
	gnss_off();
}

/*-----------------------------------------------------------------------*/
/* USART IDLE Detect Interrupt Handler                                   */
/*-----------------------------------------------------------------------*/

void USART_GNSS_IRQHandler(void)
{
    /* Check for IDLE flag */
	if (USART_GetFlagStatus(USART_GNSS, USART_GNSS_FLAG) != RESET){
		/* Clear USART registers */
		volatile uint32_t tmp;
		tmp = USART_GetITStatus(USART_GNSS, USART_GNSS_IT);
		tmp = USART_ReceiveData(USART_GNSS);
		(void)tmp;
		/* Disable DMA RX Stream */
		DMA_Cmd(DMA_Stream_USART_GNSS_RX, DISABLE);
		while (DMA_GetCmdStatus(DMA_Stream_USART_GNSS_RX ) != DISABLE) { ; }
	}
}

/*-----------------------------------------------------------------------*/
/* DMA Transfer Complete interrupt Handler                               */
/*-----------------------------------------------------------------------*/
#ifdef STM32_GNSS_USE_DMA
void DMA_USART_RX_IRQHandler (void)
{
	/* Test on DMA Stream Transfer Complete interrupt */
	if (DMA_GetFlagStatus(DMA_Stream_USART_GNSS_RX, DMA_FLAG_USART_GNSS_TC_RX) != RESET)
	{
		gnss_Length = DMA_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA_Stream_USART_GNSS_RX);
		STM_EVAL_LEDOn(LED6);
		zero_gnss_memory();
		if (gnss_Length > 1) for(int i = 0; i < gnss_Length; i++) GNSS_LOG_Buffer[i] = DMA_RX_Buffer[i];
		STM_EVAL_LEDOff(LED6);
		//DMA_InitTypeDef DMA_InitStructure;
		//DMA_InitStructure.DMA_Memory0BaseAddr = (DWORD)DMA_RX_Buffer;
		//DMA_InitStructure.DMA_BufferSize = DMA_RX_BUFFER_SIZE;
		//DMA_Init(DMA_Stream_USART_GNSS_RX, &DMA_InitStructure);
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA_Stream_USART_GNSS_RX, DMA_Stream_USART_IT);
		/* Enable DMA transfer */
		DMA_Cmd(DMA_Stream_USART_GNSS_RX, ENABLE);
		while (DMA_GetCmdStatus(DMA_Stream_USART_GNSS_RX ) != ENABLE) { ; }
	}
}
#endif
