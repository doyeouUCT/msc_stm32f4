/*
 * sensor.c
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#include "sensorio.h"

/*-----------------------------------------------------------------------
 Module Private Functions and Variables
 ------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* Transmit/Receive a byte via SPI  (Platform dependent) */
/*-----------------------------------------------------------------------*/
static
uint8_t stm32_sensor_rw( uint8_t out )
{
	/* Loop while DR register in not empty */
	//while (SPI_I2S_GetFlagStatus(SPI_BOSCH, SPI_I2S_FLAG_TXE) == RESET) { ; }

	/* Send byte through the SPI peripheral */
	SPI_I2S_SendData(SPI_BOSCH, out);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI_BOSCH, SPI_I2S_FLAG_RXNE) == RESET) { ; }

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI_BOSCH);
}

/*-----------------------------------------------------------------------*/
/* Transmit a byte to BOSCH sensor via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

#define xmit_sensor(dat)  stm32_sensor_rw(dat)

/*-----------------------------------------------------------------------*/
/* Receive a byte from BOSCH sensor via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
uint8_t rcvr_sensor (void)
{
	return stm32_sensor_rw(0xff);
}

/* Alternative macro to receive data fast */
#define rcvr_sensor_m(dst)  *(dst)=stm32_sensor_rw(0xff)

/*-----------------------------------------------------------------------*/
/* Deselect the BOSCH sensor and release SPI bus                         */
/*-----------------------------------------------------------------------*/

static
void release_sensor (void)
{
	ACC_DESELECT();
	GYRO_DESELECT();
	MAG_DESELECT();
	BARO_DESELECT();
	rcvr_sensor();
}

/*-----------------------------------------------------------------------*/
/* Initialize the BOSCH sensor and release SPI bus                       */
/*-----------------------------------------------------------------------*/

static
void sensor_on (void){

	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t dummyread;

	/* Enable GPIO clock for CSs */
	RCC_AHBPeriphClockCmd_SPI_BOSCH_CS(RCC_AHBPeriph_GPIO_BOSCH_CS, ENABLE);
	/* Enable SPI clock, SPI1: APB2, SPI2: APB1 */
	RCC_APBPeriphClockCmd_SPI_BOSCH(RCC_APBPeriph_SPI_BOSCH, ENABLE);

	/* SPI GPIO Configuration --------------------------------------------------*/

	/* Configure SPI Chip Select pin */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* SPI Accelerometer CS pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_ACC_CS;
	GPIO_Init(GPIO_BOSCH_CS, &GPIO_InitStructure);

	/* SPI Gyroscope CS pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_GYRO_CS;
	GPIO_Init(GPIO_BOSCH_CS, &GPIO_InitStructure);

	/* SPI Magnetometer CS pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_MAG_CS;
	GPIO_Init(GPIO_BOSCH_CS, &GPIO_InitStructure);

	/* SPI Barometer CS pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_BARO_CS;
	GPIO_Init(GPIO_BOSCH_CS, &GPIO_InitStructure);

	/* De-select the sensor: Chip Select high */
	ACC_DESELECT();
	GYRO_DESELECT();
	MAG_DESELECT();
	BARO_DESELECT();

	/* Configure SPI pins: SCK and MOSI with default alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_BOSCH_SCK;
	GPIO_Init(GPIO_SPI_BOSCH, &GPIO_InitStructure);

	/* SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_BOSCH_MOSI;
	GPIO_Init(GPIO_SPI_BOSCH, &GPIO_InitStructure);

	/* Configure MISO as Input with internal pull-up */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_BOSCH_MISO;
	GPIO_Init(GPIO_SPI_BOSCH, &GPIO_InitStructure);

	/* Connect SPI pins to AF5 */
	GPIO_PinAFConfig(GPIO_SPI_BOSCH, GPIO_Pin_SPI_BOSCH_SCK_Src, GPIO_AF_SPI_BOSCH );	// SCK
	GPIO_PinAFConfig(GPIO_SPI_BOSCH, GPIO_Pin_SPI_BOSCH_MOSI_Src, GPIO_AF_SPI_BOSCH );	// MOSI
	GPIO_PinAFConfig(GPIO_SPI_BOSCH, GPIO_Pin_SPI_BOSCH_MISO_Src, GPIO_AF_SPI_BOSCH );	// MISO

	/* SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_SPI_BOSCH; // 72000kHz/16 = 4500kHz < 10000kHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI_BOSCH, &SPI_InitStructure);
	SPI_CalculateCRC(SPI_BOSCH, DISABLE);
	SPI_Cmd(SPI_BOSCH, ENABLE);

	/* drain SPI */
	while (SPI_I2S_GetFlagStatus(SPI_BOSCH, SPI_I2S_FLAG_TXE) == RESET) { ; }
	dummyread = SPI_I2S_ReceiveData(SPI_BOSCH);
	if (dummyread) { ; }
}

/*-----------------------------------------------------------------------*/
/* De-initialize the BOSCH sensor and release SPI bus                    */
/*-----------------------------------------------------------------------*/

static
void sensor_off(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	SPI_I2S_DeInit(SPI_BOSCH );
	SPI_Cmd(SPI_BOSCH, DISABLE);
	RCC_APBPeriphClockCmd_SPI_BOSCH(RCC_APBPeriph_SPI_BOSCH, DISABLE);

	/* All SPI-Pins to input with weak internal pull-downs */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_BOSCH_SCK | GPIO_Pin_SPI_BOSCH_MISO | GPIO_Pin_SPI_BOSCH_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIO_SPI_BOSCH, &GPIO_InitStructure);
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from BOSCH sensor                                        */
/*-----------------------------------------------------------------------*/

static
uint8_t rcvr_data (
	uint8_t addr,		/* Address of the register to read */
	SENSOR sensor	/* Sensor type */
)
{
	switch(sensor){
	case(ACCEL)	:	ACC_SELECT();	break;
	case(GYRO)	:	GYRO_SELECT();	break;
	case(MAG)	:	MAG_SELECT();	break;
	default		:	;
	}

	uint8_t token = 0x80 | addr;				/* Read token */
	xmit_sensor(token);
	token = rcvr_sensor();					/* Return with success */

	switch(sensor){
	case(ACCEL)	:	ACC_DESELECT();		break;
	case(GYRO)	:	GYRO_DESELECT();	break;
	case(MAG)	:	MAG_DESELECT();		break;
	default		:	;
	}

	return token;
}

static
SSTATUS rcvr_data_m (
	uint8_t *buff,		/* Data buffer to store received data */
	uint8_t addr,		/* Address of the register to read */
	SENSOR sensor,		/* Sensor type */
	uint8_t length		/* Length of Buffer */
)
{
	switch(sensor){
	case(ACCEL)	:	ACC_SELECT();	break;
	case(GYRO)	:	GYRO_SELECT();	break;
	case(MAG)	:	MAG_SELECT();	break;
	default		:	;
	}

	uint8_t token = 0x80 | addr;				/* Read token */
	xmit_sensor(token);
	for(int i = 0; i < length; i++){
		rcvr_sensor_m(buff++);
	}

	switch(sensor){
	case(ACCEL)	:	ACC_DESELECT();		break;
	case(GYRO)	:	GYRO_DESELECT();	break;
	case(MAG)	:	MAG_DESELECT();		break;
	default		:	;
	}

	return SENS_OK;					/* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

static
SSTATUS xmit_data (
	uint8_t out,		/* Data buffer which contains data to be written */
	uint8_t addr,		/* Address of the register to write */
	SENSOR sensor	/* Sensor type */
)
{
	switch(sensor){
	case(ACCEL)	:	ACC_SELECT();	break;
	case(GYRO)	:	GYRO_SELECT();	break;
	case(MAG)	:	MAG_SELECT();	break;
	default		:	;
	}


	uint8_t token = 0x7F & addr;		/* Write token */
	xmit_sensor(token);
	xmit_sensor(out);

	switch(sensor){
	case(ACCEL)	:	ACC_DESELECT();		break;
	case(GYRO)	:	GYRO_DESELECT();	break;
	case(MAG)	:	MAG_DESELECT();		break;
	default		:	;
	}

	uint8_t sample = rcvr_data(addr, sensor);

	if(sample != out)		/* Check if written correctly */
		return SENS_ERROR;

	return SENS_OK;
}

/*-----------------------------------------------------------------------

 Public Functions

 ------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize BOSCH Sensor                                               */
/*-----------------------------------------------------------------------*/

SSTATUS sensor_initialize (
	volatile SCONFIG *f
	)
{
	sensor_on();						/* Initialize interface */
	sensor_buffer[0]	=	S_BUFFER0;	/* Initialize varialbe */
	sensor_buffer[1]	=	S_BUFFER1;
	sensor_buffer[2]	=	S_BUFFER2;
	sensor_buffer[3]	=	S_BUFFER3;
	sensor_crnt_buffer	=	0;
	sensor_crnt_pos		=	0;
	/* Essential delay > (3*)30000 us */
	for(int counter = 0; counter < 50000; counter++) {;}
	uint8_t id;
#ifdef BMX055_ACC_ID_CHECK
	id = rcvr_data(BMX055_ACC_CHIP_ID, ACCEL);
	if(id != BMX055_ACC_ID)	return SENS_IDERR;
#endif
#ifdef BMX055_GYRO_ID_CHECK
	id = rcvr_data(BMX055_GYRO_CHIP_ID, GYRO);
	if(id != BMX055_GYRO_ID)	return SENS_IDERR;
#endif
#ifdef BMX055_MAG_ID_CHECK
	xmit_data(MAG_PWR_CR1_POWER_CONTROL_BIT, BMX055_MAG_PWR_CR1) != SENS_OK);
	id = rcvr_data(BMX055_MAG_CHIP_ID, MAG);
	if(id != BMX055_MAG_ID)	return SENS_IDERR;
#endif

	/* Accelerometer configuration: range, rate & interrupt */
	if(xmit_data(f->accel_range, BMX055_ACC_PMU_RANGE, ACCEL) != SENS_OK)	return SENS_CONFERR;
	else{
		if(xmit_data(f->accel_rate, BMX055_ACC_PMU_BW, ACCEL) != SENS_OK)	return SENS_CONFERR;
	}
#ifdef BMX055_ACC_DATA_READY
	if(xmit_data(ACC_INT_MAP_1_DATA_1, BMX055_ACC_INT_MAP_1, ACCEL) != SENS_OK)	return SENS_INTCONFERR;
	else{
		if(xmit_data(((~ACC_INT_OUT_CTRL_1_OD) & ACC_INT_OUT_CTRL_RESET), BMX055_ACC_INT_OUT_CTRL, ACCEL) != SENS_OK)	return SENS_INTCONFERR;
		else{
			if(xmit_data(ACC_INT_EN_1_DATA, BMX055_ACC_INT_EN_1, ACCEL) != SENS_OK)	return SENS_INTCONFERR;
		}
	}
#endif

	/* Gyroscope configuration: range, rate & interrupt */
	if(xmit_data(f->gyro_range, BMX055_GYRO_RANGE, GYRO) != SENS_OK)	return SENS_CONFERR;
	else{
		if(xmit_data(f->gyro_rate, BMX055_GYRO_BW, GYRO) != SENS_OK)	return SENS_CONFERR;
	}
#ifdef BMX055_GYRO_DATA_READY
	if(xmit_data(GYRO_INT_MAP_1_INT1_DATA, BMX055_GYRO_INT_MAP_1, GYRO) != SENS_OK)	return SENS_INTCONFERR;
	else{
		if(xmit_data(((~GYRO_INT_EN_1_INT1_OD) & BMX055_GYRO_INT_EN_1_RESET), BMX055_GYRO_INT_EN_1, GYRO) != SENS_OK)	return SENS_INTCONFERR;
		else{
			if(xmit_data(GYRO_INT_EN_0_DATA_EN, BMX055_GYRO_INT_EN_0, GYRO) != SENS_OK)	return SENS_INTCONFERR;
		}
	}
#endif
#ifdef BMX055_MAG_DATA_READY
		/* Magnetometer configuration: range, rate & interrupt */
	if(xmit_data(f->mag_rate, BMX055_MAG_PWR_CR2, MAG) != SENS_OK)	return SENS_CONFERR;
	else{
		if(xmit_data(f->mag_range_xy, BMX055_MAG_REP_XY, MAG) != SENS_OK)	return SENS_CONFERR;
		else{
			if(xmit_data(f->mag_range_z, BMX055_MAG_REP_Z, MAG) != SENS_OK)	return SENS_CONFERR;
		}
	}

	if(xmit_data((MAG_INT_CR2_DATA_READY_PIN_EN|
					MAG_INT_CR2_DR_POLARITY|
					MAG_INT_CR2_INTERRUPT_LATCH|
					MAG_INT_CR2_INTERRUPT_POLARITY, MAG), BMX055_MAG_PWR_CR2) != SENS_OK)
		return SENS_INTCONFERR;
#endif

	return SENS_OK;
}

SSTATUS sensor_deinitialize (void)
{
	release_sensor();
	sensor_off();
	return SENS_OK;
}

SSTATUS accel_read (
	uint8_t *buff,
	uint8_t data_length)
{
#ifdef BMX055_ACC_ID_CHECK
	if(rcvr_data(BMX055_ACC_CHIP_ID, ACCEL) != BMX055_ACC_ID)	return SENS_NOTRDY;
#endif
	if(rcvr_data_m(buff, BMX055_ACC_D_X_LSB, ACCEL, data_length) != SENS_OK)
		return SENS_ERROR;
	return SENS_OK;
}

SSTATUS gyro_read (
	uint8_t *buff,
	uint8_t data_length)
{
#ifdef BMX055_GYRO_ID_CHECK
	if(rcvr_data(BMX055_GYRO_CHIP_ID, GYRO) != BMX055_GYRO_ID)	return SENS_NOTRDY;
#endif
	if(rcvr_data_m(buff, BMX055_GYRO_RATE_X_LSB, GYRO, data_length) != SENS_OK)
		return SENS_ERROR;
	return SENS_OK;
}

SSTATUS mag_read (
	uint8_t *buff,
	uint8_t data_length)
{
	if(rcvr_data_m(buff, BMX055_MAG_XOUT_LSB, MAG, data_length) != SENS_OK)
		return SENS_ERROR;
	return SENS_OK;
}
