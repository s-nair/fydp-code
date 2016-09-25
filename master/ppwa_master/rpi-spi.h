#ifndef RPI_SPI_H
#define RPI_SPI_H

/* The base address of the GPIO peripheral (ARM Physical Address) */
#ifdef RPI2
    #define SPI_BASE       0x3F204000UL
	#define SPI_EN_BASE    0x3F215000UL
#else
    #define SPI_BASE       0x20204000UL
	#define SPI_EN_BASE    0x20215000UL	
#endif

#define ENABLE		1
#define SPI_ENABLE	1

/////////////////////
/* SPI ADDRESS MAP */
/////////////////////

#define SPI_CTRL	0
#define SPI_FIFO	1
#define SPI_CLK		2
#define SPI_DLEN	3
#define SPI_LTOH	4
#define SPI_DC		5

/////////////////////
/* CS Register */
/////////////////////

/* RW */
#define SPI_CSPOL0	21
#define SPI_CSPOL1	22
#define SPI_CSPOL	6
#define SPI_TA		7
#define SPI_CLEAR	4
#define SPI_CPOL	3
#define SPI_CPHA	2
#define SPI_CS		0

/* RO */
#define SPI_RXF		20
#define SPI_RXR		19
#define SPI_TXD		18
#define SPI_RXD		17
#define SPI_DONE	16

/////////////////////
/* FIFO Register */
/////////////////////
#define SPI_DATA	0

/////////////////////
/* CLK Register */
/////////////////////
#define SPI_CDIV	0

#endif