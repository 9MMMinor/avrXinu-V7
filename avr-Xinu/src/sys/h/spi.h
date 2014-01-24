/*
 *  spi.h
 *  Xinu_Min
 *
 *  Created by Michael Minor on 3/16/10.
 *  Copyright (c) 2010. All rights reserved.
 *
 */
 
#include <avr/io.h>

/* Vectors */
/* SPI_STC_vect   SPI Serial Transfer Complete */

/* Registers */

#define SPI_DDR DDRB
#define SPI_PORT PORTB

/* SPI pins */

#define MOSI	PB5  /* Port B Pin 5 */
#define MISO	PB6
#define SCK		PB7
#define SSBAR	PB4

/* Data direction bits for SPI Pins */

#define DD_MOSI		DDB5
#define DD_MISO		DDB6
#define DD_SCK		DDB7
#define DD_SSBAR	DDB4

/* SPI register set */
typedef struct spi_csr	{
	volatile unsigned char SPICR;			/*  SPI Control Register		*/
	volatile unsigned char SPISR;			/*  SPI Status Register 		*/
	volatile unsigned char SPIDR;			/*  SPI Data Register			*/
}SPI_t;

#define SPI0 (*(SPI_t *) SPCR)				/* SPI0.SPICR for example		*/
#define SPI_ptr ((SPI_t *) SPCR)			/* SPI_ptr->SPICR for example	*/


/* Driver States */

#define SPI_INIT			1
#define SPI_WRITE			2
#define SPI_READ			3
#define SPI_READ_EXTRA_BYTE	4
#define SPI_READ_SKIP_FIRST	5
#define SPI_PUT				6
#define SPI_DONE			7

/* SPI Control Functions */

#define SPI_SetModeToMaster	1
#define SPI_SetModeToSlave	2

/* SPI byte constants */

#define SPI_DUMMY 0xff

struct spiblk {
	char iomode;		/* Master or Slave */
	char iostate;		/* i/o state */
	int proc;			/* process to resume after block read or write */
	int count;			/* number of bytes to read or write */
	char *next;			/* next buffer location */
	int func;			/* command byte = op | (address & ADDR_MASK) */
};

#ifndef Nspi
#define Nspi 1
#endif

extern struct spiblk spi[];

