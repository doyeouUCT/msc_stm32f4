/*
 * diskio.h
 *
 * Created on	:	Feb 23, 2018
 * Author		:	Do Yeou Ku
 * Description	:	MMC/SDSC/SDHC SPI control module
 * Reference	:	(C)ChaN, 2007 & (C) Martin Thomas, 2010
 *
 */

/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for STM32 Version 1.1.6    */
/* (C) Martin Thomas, 2010 - based on the AVR MMC module (C)ChaN, 2007   */
/*-----------------------------------------------------------------------*/


#include "diskio.h"

// #define STM32_SD_USE_DMA
#ifdef STM32_SD_USE_DMA
// #warning "Information only: using DMA"
#pragma message "*** Using DMA ***"
#endif

/* set to 1 to provide a disk_ioctrl function even if not needed by the FatFs */
#define STM32_SD_DISK_IOCTRL_FORCE      0

/* Boolean defined. */
#define BOOL char
#define TRUE 1
#define FALSE 0

#define USE_EK_STM32F

/* PWR pin defined. TODO: check
#define RCC_APB2Periph_GPIO_PWR  RCC_APB2Periph_GPIOD
#define GPIO_PWR                 GPIOD
#define GPIO_Pin_PWR             GPIO_Pin_10
#define GPIO_Mode_PWR            GPIO_Mode_Out_OD
*/
// pull-up resistor at power FET

/* Disk connection pin defined. */
#define CARD_SUPPLY_SWITCHABLE   0
#define SOCKET_WP_CONNECTED      0
#define SOCKET_CP_CONNECTED      0

/* SPI connection pin defined. */
#define RCC_AHBPeriphClockCmd_SPI_CS	RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_GPIO_CS   		RCC_AHB1Periph_GPIOB
#define GPIO_CS                 		GPIOB
#define GPIO_Pin_CS              		GPIO_Pin_12

#define RCC_APBPeriphClockCmd_SPI_SD	RCC_APB1PeriphClockCmd
#define RCC_APBPeriph_SPI_SD			RCC_APB1Periph_SPI2
#define SPI_SD                   		SPI2
#define GPIO_SPI_SD              		GPIOB
#define GPIO_Pin_SPI_SD_SCK      		GPIO_Pin_13
#define GPIO_Pin_SPI_SD_MISO     		GPIO_Pin_14
#define GPIO_Pin_SPI_SD_MOSI     		GPIO_Pin_15

#define GPIO_Pin_SPI_SD_SCK_Src			GPIO_PinSource13
#define GPIO_Pin_SPI_SD_MISO_Src		GPIO_PinSource14
#define GPIO_Pin_SPI_SD_MOSI_Src		GPIO_PinSource15

#define GPIO_AF_SPI_SD					GPIO_AF_SPI2

/* SPI full-speed : 72MHz/4 TODO: check*/
#define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_8

/* DMA connection defined. */
#define RCC_AHBPeriph_DMA		RCC_AHB1Periph_DMA1
#define DMA_Channel_SPI_SD_RX	DMA_Channel_0
#define DMA_Channel_SPI_SD_TX	DMA_Channel_0
#define DMA_Stream_SPI_SD_RX	DMA1_Stream3
#define DMA_Stream_SPI_SD_TX	DMA1_Stream4
#define DMA_FLAG_SPI_SD_TC_RX	DMA_FLAG_TCIF3
#define DMA_FLAG_SPI_SD_TC_TX	DMA_FLAG_TCIF4
#define DMA_SPI_RX_IRQn			DMA1_Stream3_IRQn
#define DMA_SPI_TX_IRQn			DMA1_Stream4_IRQn
#define DMA_SPI_RX_IRQHandler	DMA1_Stream3_IRQHandler
#define DMA_SPI_TX_IRQHandler	DMA1_Stream4_IRQHandler

/* MMC/SDC command defined. */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card-Select control defined. (Platform dependent) */
#define SELECT()        GPIO_ResetBits(GPIO_CS, GPIO_Pin_CS)    /* MMC CS = L */
#define DESELECT()      GPIO_SetBits(GPIO_CS, GPIO_Pin_CS)      /* MMC CS = H */

/* Socket contacts defined. (Dummy values) */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch */
#define SOCKINS		0			/* Card detect switch */

#if (_MAX_SS != 512) || (_FS_READONLY == 0) || (STM32_SD_DISK_IOCTRL_FORCE == 1)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif

/*--------------------------------------------------------------------------
 Module Private Functions and Variables
 ---------------------------------------------------------------------------*/

static const DWORD socket_state_mask_cp = (1 << 0);
static const DWORD socket_state_mask_wp = (1 << 1);

/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* 100Hz decrement timers */
static volatile DWORD Timer1, Timer2;

/* Card type flags */
static BYTE CardType;

enum speed_setting { INTERFACE_SLOW, INTERFACE_FAST };

static void interface_speed( enum speed_setting speed )
{
	DWORD tmp;

	tmp = SPI_SD->CR1;
	if ( speed == INTERFACE_SLOW ) {
		/* Set slow clock (100k-400k) */
		tmp = ( tmp | SPI_BaudRatePrescaler_256 );
	} else {
		/* Set fast clock (depends on the CSD) */
		tmp = ( tmp & ~SPI_BaudRatePrescaler_256 ) | SPI_BaudRatePrescaler_SPI_SD;
	}
	SPI_SD->CR1 = tmp;
}

#if SOCKET_WP_CONNECTED
/* Socket's Write-Protection Pin: high = write-protected, low = writable */

static void socket_wp_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure I/O for write-protect */
	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIO_WP, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_WP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_WP;
	GPIO_Init(GPIO_WP, &GPIO_InitStructure);
}

static DWORD socket_is_write_protected(void)
{
	return ( GPIO_ReadInputData(GPIO_WP) & GPIO_Pin_WP ) ? socket_state_mask_wp : 0;
}

#else

static void socket_wp_init(void)
{
	return;
}


static inline DWORD socket_is_write_protected(void)
{
	return 0; /* fake not protected */
}

#endif /* SOCKET_WP_CONNECTED */

#if SOCKET_CP_CONNECTED
/* Socket's Card-Present Pin: high = socket empty, low = card inserted */

static void socket_cp_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure I/O for card-present */
	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIO_CP, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_CP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_CP;
	GPIO_Init(GPIO_CP, &GPIO_InitStructure);
}

static inline DWORD socket_is_empty(void)
{
	return ( GPIO_ReadInputData(GPIO_CP) & GPIO_Pin_CP ) ? socket_state_mask_cp : FALSE;
}

#else

static void socket_cp_init(void)
{
	return;
}

static inline DWORD socket_is_empty(void)
{
	return 0; /* fake inserted */
}

#endif /* SOCKET_CP_CONNECTED */

#if CARD_SUPPLY_SWITCHABLE

static void card_power(BOOL on)		/* switch FET for card-socket VCC */
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Turn on GPIO for power-control pin connected to FET's gate */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_PWR, ENABLE);
	/* Configure I/O for Power FET */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_PWR;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_PWR;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PWR, &GPIO_InitStructure);
	if (on) {
		GPIO_ResetBits(GPIO_PWR, GPIO_Pin_PWR);
	} else {
		/* Chip select internal pull-down (to avoid parasite powering) */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
		GPIO_Init(GPIO_CS, &GPIO_InitStructure);

		GPIO_SetBits(GPIO_PWR, GPIO_Pin_PWR);
	}
}

#if (STM32_SD_DISK_IOCTRL == 1)
static int chk_power(void)		/* Socket power state: 0=off, 1=on */
{
	if ( GPIO_ReadOutputDataBit(GPIO_PWR, GPIO_Pin_PWR) == Bit_SET ) {
		return 0;
	} else {
		return 1;
	}
}
#endif

#else

static void card_power(BYTE on)
{
	on=on;
}

#if (STM32_SD_DISK_IOCTRL == 1)
static int chk_power(void)
{
	return 1; /* fake powered */
}
#endif

#endif /* CARD_SUPPLY_SWITCHABLE */

/*-----------------------------------------------------------------------*/
/* Transmit/Receive a byte to MMC via SPI  (Platform dependent)          */
/*-----------------------------------------------------------------------*/
static
BYTE stm32_spi_rw( BYTE out )
{
	/* Loop while DR register in not empty */
	/// not needed: while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_TXE) == RESET) { ; }

	/* Send byte through the SPI peripheral */
	SPI_I2S_SendData(SPI_SD, out);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_RXNE) == RESET) { ; }

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI_SD);
}

/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

#define xmit_spi(dat)  stm32_spi_rw(dat)

/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static BYTE rcvr_spi (void)
{
	return stm32_spi_rw(0xff);
}

/* Alternative macro to receive data fast */
#define rcvr_spi_m(dst)  *(dst)=stm32_spi_rw(0xff)

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static BYTE wait_ready (void)
{
	BYTE res;


	Timer2 = 50;	/* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do
		res = rcvr_spi();
	while ((res != 0xFF) && Timer2);

	return res;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static void release_spi (void)
{
	DESELECT();
	rcvr_spi();
}

#ifdef STM32_SD_USE_DMA
/*-----------------------------------------------------------------------*/
/* Transmit/Receive Block using DMA (Platform dependent. STM32 here)     */
/*-----------------------------------------------------------------------*/
static
void stm32_dma_transfer(
	BOOL receive,		/* FALSE for buff->SPI, TRUE for SPI->buff               */
	const BYTE *buff,	/* receive TRUE  : 512 byte data block to be transmitted
						   receive FALSE : Data buffer to store received data    */
	UINT btr 			/* receive TRUE  : Byte count (must be multiple of 2)
						   receive FALSE : Byte count (must be 512)              */
)
{

	DMA_InitTypeDef DMA_InitStructure;
	WORD rw_workbyte[] = { 0xffff };

	/* De-initialize DMA RX & TX Stream */
	DMA_DeInit(DMA_Stream_SPI_SD_RX);
	while (DMA_GetCmdStatus(DMA_Stream_SPI_SD_RX ) != DISABLE) { ; }
	DMA_DeInit(DMA_Stream_SPI_SD_TX);
	while (DMA_GetCmdStatus(DMA_Stream_SPI_SD_TX ) != DISABLE) { ; }

	/* shared DMA configuration values */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (DWORD)(&(SPI_SD->DR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;

	DMA_InitStructure.DMA_BufferSize = btr;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;


	if ( receive ) {
		/* DMA1 channel0 configuration SPI2 RX */
		DMA_InitStructure.DMA_Channel = DMA_Channel_SPI_SD_RX;
		DMA_InitStructure.DMA_Memory0BaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_Init(DMA_Stream_SPI_SD_RX, &DMA_InitStructure);

		/* DMA1 channel0 configuration SPI2 TX */
		DMA_InitStructure.DMA_Channel = DMA_Channel_SPI_SD_TX;
		DMA_InitStructure.DMA_Memory0BaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		DMA_Init(DMA_Stream_SPI_SD_TX, &DMA_InitStructure);
	}
	else {
		#if _FS_READONLY == 0
		/* DMA1 channel0 configuration SPI2 RX */
		DMA_InitStructure.DMA_Channel = DMA_Channel_SPI_SD_RX;
		DMA_InitStructure.DMA_Memory0BaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		DMA_Init(DMA_Stream_SPI_SD_RX, &DMA_InitStructure);

		/* DMA1 channel0 configuration SPI2 TX */
		DMA_InitStructure.DMA_Channel = DMA_Channel_SPI_SD_TX;
		DMA_InitStructure.DMA_Memory0BaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_Init(DMA_Stream_SPI_SD_TX, &DMA_InitStructure);
		#endif
	}

	/**
	DMA_ITConfig(DMA_Stream_SPI_SD_TX, DMA_IT_TC, ENABLE);

	// enable the interrupt in the NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA_SPI_TX_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	**/

	/* Enable DMA RX & TX Stream */
	DMA_Cmd(DMA_Stream_SPI_SD_RX, ENABLE);
	while (DMA_GetCmdStatus(DMA_Stream_SPI_SD_RX ) != ENABLE) { ; }
	DMA_Cmd(DMA_Stream_SPI_SD_TX, ENABLE);
	while (DMA_GetCmdStatus(DMA_Stream_SPI_SD_TX ) != ENABLE) { ; }

	/* Enable SPI RX & TX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx, ENABLE);
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Wait until DMA1_Channel 0 Transfer Complete */
	/* Not needed to use CPU */
	while (DMA_GetFlagStatus(DMA_Stream_SPI_SD_RX, DMA_FLAG_SPI_SD_TC_RX)==RESET);
	while (DMA_GetFlagStatus(DMA_Stream_SPI_SD_TX, DMA_FLAG_SPI_SD_TC_TX)==RESET);

	/* Disable DMA RX & TX Stream */
	DMA_Cmd(DMA_Stream_SPI_SD_RX, DISABLE);
	DMA_Cmd(DMA_Stream_SPI_SD_TX, DISABLE);

	/* Disable SPI RX & TX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx, DISABLE);
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Tx, DISABLE);

}
#endif /* STM32_SD_USE_DMA */


/*-----------------------------------------------------------------------*/
/* Power Control and interface-initialization (Platform dependent)       */
/*-----------------------------------------------------------------------*/

static
void power_on (void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	volatile BYTE dummyread;

	/* Enable GPIO clock for CS */
	RCC_AHBPeriphClockCmd_SPI_CS(RCC_AHBPeriph_GPIO_CS, ENABLE);
	/* Enable SPI clock, SPI1: APB2, SPI2: APB1 */
	RCC_APBPeriphClockCmd_SPI_SD(RCC_APBPeriph_SPI_SD, ENABLE);

	card_power(1);
	socket_cp_init();
	socket_wp_init();

	for (Timer1 = 25; Timer1; );	/* Wait for 250ms */

	/* SPI GPIO Configuration --------------------------------------------------*/

	/* Configure SPI Chip Select pin */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* SPI CS pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
	GPIO_Init(GPIO_CS, &GPIO_InitStructure);

	/* De-select the Card: Chip Select high */
	DESELECT();

	/* Configure SPI pins: SCK and MOSI with default alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_SD_SCK;
	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);

	/* SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_SD_MOSI;
	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);

	/* Configure MISO as Input with internal pull-up */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_SD_MISO;
	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);

	/* Connect SPI pins to AF5 */
	GPIO_PinAFConfig(GPIO_SPI_SD, GPIO_Pin_SPI_SD_SCK_Src, GPIO_AF_SPI_SD ); // SCK
	GPIO_PinAFConfig(GPIO_SPI_SD, GPIO_Pin_SPI_SD_MOSI_Src, GPIO_AF_SPI_SD ); // MOSI
	GPIO_PinAFConfig(GPIO_SPI_SD, GPIO_Pin_SPI_SD_MISO_Src, GPIO_AF_SPI_SD ); // MISO

	/* SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // 72000kHz/256=281kHz < 400kHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI_SD, &SPI_InitStructure);
	SPI_CalculateCRC(SPI_SD, DISABLE);
	SPI_Cmd(SPI_SD, ENABLE);

	/* drain SPI */
	while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_TXE) == RESET) { ; }
	dummyread = SPI_I2S_ReceiveData(SPI_SD);
	if (dummyread) { ; }

#ifdef STM32_SD_USE_DMA
	/* enable DMA clock */
	RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_DMA, ENABLE);
#endif
}

static
void power_off(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	if (!(Stat & STA_NOINIT)) {
		SELECT();
		wait_ready();
		release_spi();
	}

	SPI_I2S_DeInit(SPI_SD );
	SPI_Cmd(SPI_SD, DISABLE);
	RCC_APBPeriphClockCmd_SPI_SD(RCC_APBPeriph_SPI_SD, DISABLE);

	/* All SPI-Pins to input with weak internal pull-downs */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_SD_SCK | GPIO_Pin_SPI_SD_MISO | GPIO_Pin_SPI_SD_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);

	card_power(0);

	Stat |= STA_NOINIT; /* Set STA_NOINIT */
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/
static
BOOL rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 10;
	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);
	if(token != 0xFE) return FALSE;	/* If not valid data token, return with error */

#ifdef STM32_SD_USE_DMA
	stm32_dma_transfer( TRUE, buff, btr );
#else
	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
#endif /* STM32_SD_USE_DMA */

	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return TRUE;					/* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0
static
BOOL xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp;
#ifndef STM32_SD_USE_DMA
	BYTE wc;
#endif

	if (wait_ready() != 0xFF) return FALSE;

	xmit_spi(token);					/* transmit data token */
	if (token != 0xFD) {				/* Is data token */

#ifdef STM32_SD_USE_DMA
		stm32_dma_transfer( FALSE, buff, 512 );
#else
		wc = 0;
		do {							/* transmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
#endif 									/* STM32_SD_USE_DMA */

		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Receive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (wait_ready() != 0xFF) {
		return 0xFF;
	}

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */

	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}

/*--------------------------------------------------------------------------

 Public Functions

 ---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	BYTE n, cmd, ty, ocr[4];

	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on and initialize interface */
	interface_speed(INTERFACE_SLOW);
	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 100;						/* Initialization timeout of 1000 milliseconds */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 response */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at VDD range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDSC */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	release_spi();

	if (ty) {			/* Initialization succeeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		interface_speed(INTERFACE_FAST);
	} else {			/* Initialization failed */
		power_off();
	}

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if (send_cmd(CMD17, sector) == 0)	{ /* READ_SINGLE_BLOCK */
			if (rcvr_datablock(buff, 512)) {
				count = 0;
			}
		}
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) {
					break;
				}
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0

DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if (STM32_SD_DISK_IOCTRL == 1)
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;

	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER) {
		switch (*ptr) {
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			if (chk_power())
				power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			power_on();				/* Power on */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = (BYTE)chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else {
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC version 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC version 1.XX or MMC*/
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & CT_SD2) {	/* SDC version 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC version 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDC version 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD status as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void disk_timerproc (void)
{
	static DWORD pv;
	DWORD ns;
	BYTE n, s;


	n = Timer1;                /* 100Hz decrement timers */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	ns = pv;
	pv = socket_is_empty() | socket_is_write_protected();	/* Sample socket switch */

	if (ns == pv) {                         /* Have contacts stabled? */
		s = Stat;

		if (pv & socket_state_mask_wp)      /* WP is H (write protected) */
			s |= STA_PROTECT;
		else                                /* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pv & socket_state_mask_cp)      /* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else                                /* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}
