/*
 * sensor.h
 *
 *  Created on: Mar 5, 2018
 *      Author: Do Yeou Ku
 */

#ifndef TASKS_SENSORIO_H_
#define TASKS_SENSORIO_H_

#include "stm32f4xx.h"
#include "bmx055.h"
#include "bmp280.h"

#define RTK_IMU_SENSOR

#ifdef RTK_IMU_SENSOR
/* ID check enable */
#define BMX055_ACC_ID_CHECK
#define BMX055_GYRO_ID_CHECK
//#define BMX055_MAG_ID_CHECL
/* Configure enable */
#define BMX055_ACC_DATA_READY
#define BMX055_GYRO_DATA_READY
//#define BMX055_MAG_DATA_READY
#endif

/* Receive buffer for BMX055 */
#define MAX_DATA_LENGTH		20
#define ACCEL_DATA_LENGTH	7
#define GYRO_DATA_LENGTH	6
#define MAG_DATA_LENGTH		8

#define SENSOR_LOG_BUFFER_SIZE		75
uint8_t S_BUFFER0 [SENSOR_LOG_BUFFER_SIZE];
uint8_t S_BUFFER1 [SENSOR_LOG_BUFFER_SIZE];
uint8_t S_BUFFER2 [SENSOR_LOG_BUFFER_SIZE];
uint8_t S_BUFFER3 [SENSOR_LOG_BUFFER_SIZE];

uint8_t* 			sensor_buffer [4];
volatile uint8_t	sensor_crnt_buffer;
volatile int		sensor_crnt_pos;

typedef enum {
	SENSOR_LOG_OFF = 0,		/* 0: Logging Off */
	SENSOR_LOG_ON,			/* 1: Logging On */
} SLOG;

uint8_t*			sensor_address_to_log;
int					sensor_bytes_to_log;
volatile SLOG		sensor_log_on;

/* Status of sensor functions */
typedef uint8_t	SSTATUS;

typedef enum {
	ACCEL	= 0,
	GYRO	= 1,
	MAG		= 2,
} SENSOR;

/* Results of Sensor Functions */
typedef enum {
	SENS_OK = 0,		/* 0: Successful */
	SENS_ERROR,			/* 1: R/W Error */
	SENS_NOTRDY,		/* 2: Not Ready */
	SENS_PARERR,		/* 3: Invalid Parameter */
	SENS_IDERR,			/* 4: Invalid ID */
	SENS_CONFERR,		/* 5: Configuration Error */
	SENS_INTCONFERR,	/* 6: Interrupt Configuration Error */
} SRESULT;

/* Sensor configuration */
typedef struct{
	uint8_t accel_rate;
	uint8_t accel_range;
	uint8_t gyro_rate;
	uint8_t gyro_range;
	uint8_t mag_rate;
	uint8_t mag_range_xy;
	uint8_t mag_range_z;
} SCONFIG;

/* Global function prototypes */
SSTATUS sensor_initialize (volatile SCONFIG*);
SSTATUS sensor_deinitialize (void);
SSTATUS accel_read (uint8_t*, uint8_t);
SSTATUS gyro_read (uint8_t*, uint8_t);
SSTATUS mag_read (uint8_t*, uint8_t);

/* SPI connection pin defined. */
#define RCC_AHBPeriphClockCmd_SPI_BOSCH_CS		RCC_AHB1PeriphClockCmd
#define RCC_AHBPeriph_GPIO_BOSCH_CS   			RCC_AHB1Periph_GPIOA
#define GPIO_BOSCH_CS	              			GPIOA
#define GPIO_Pin_ACC_CS              			GPIO_Pin_8
#define GPIO_Pin_GYRO_CS              			GPIO_Pin_10
#define GPIO_Pin_MAG_CS              			GPIO_Pin_11
#define GPIO_Pin_BARO_CS              			GPIO_Pin_11

#define RCC_APBPeriphClockCmd_SPI_BOSCH			RCC_APB2PeriphClockCmd
#define RCC_APBPeriph_SPI_BOSCH					RCC_APB2Periph_SPI1
#define SPI_BOSCH                   			SPI1
#define GPIO_SPI_BOSCH              			GPIOA
#define GPIO_Pin_SPI_BOSCH_SCK      			GPIO_Pin_5
#define GPIO_Pin_SPI_BOSCH_MISO     			GPIO_Pin_6
#define GPIO_Pin_SPI_BOSCH_MOSI     			GPIO_Pin_7

#define GPIO_Pin_SPI_BOSCH_SCK_Src				GPIO_PinSource5
#define GPIO_Pin_SPI_BOSCH_MISO_Src				GPIO_PinSource6
#define GPIO_Pin_SPI_BOSCH_MOSI_Src				GPIO_PinSource7

#define GPIO_AF_SPI_BOSCH						GPIO_AF_SPI1

#define SPI_BaudRatePrescaler_SPI_BOSCH			SPI_BaudRatePrescaler_16

/* Card-Select control defined. (Platform dependent) */
#define ACC_SELECT()        GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_ACC_CS)		/* ACC CS = L */
#define ACC_DESELECT()      GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_ACC_CS)		/* ACC CS = H */
#define GYRO_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_GYRO_CS)		/* GYRO CS = L */
#define GYRO_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_GYRO_CS)		/* GYRO CS = H */
#define MAG_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_MAG_CS)		/* MAG CS = L */
#define MAG_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_MAG_CS)		/* MAG CS = H */
#define BARO_SELECT()		GPIO_ResetBits(GPIO_BOSCH_CS, GPIO_Pin_BARO_CS)		/* BARO CS = L */
#define BARO_DESELECT()		GPIO_SetBits(GPIO_BOSCH_CS, GPIO_Pin_BARO_CS)		/* BARO CS = H */

#endif /* TASKS_SENSORIO_H_ */
