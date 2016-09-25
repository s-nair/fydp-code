/********************************************************************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 Author    : Geoffrey 
 
 Date	   : 2012

 version   :

**********************************************************************************************************************************************************/

#include "spi.h"
/*GPIO PINS
 */
volatile unsigned int *gpioGPFSEL0 =  (unsigned int *) 0x3F200000;
volatile unsigned int *gpioGPFSEL1 =  (unsigned int *) 0x3F200004;
volatile unsigned int *gpioGPSET0  =  (unsigned int *) 0x3F20001C;
volatile unsigned int *gpioGPCLR0  =  (unsigned int *) 0x3F200028;
volatile unsigned int *gpioGPLEV0  =  (unsigned int *) 0x3F200034;


/*SPI0 ADDRESS
 */
volatile unsigned int *SPI0_CONTROL =  (unsigned int *) 0x3F204000;
volatile unsigned int *SPI0_FIFO    =  (unsigned int *) 0x3F204004;
volatile unsigned int *SPI0_CLK     =  (unsigned int *) 0x3F204008;
volatile unsigned int *SPI0_DLEN    =  (unsigned int *) 0x3F20400C;
volatile unsigned int *SPI0_LTOH    =  (unsigned int *) 0x3F204010;
volatile unsigned int *SPI0_DC      =  (unsigned int *) 0x3F204014;
void led_toggle();

/* Bitfields in spi_C */
#define SPI_C_LEN_LONG		25
#define SPI_C_DMA_LEN		24
#define SPI_C_CSPOL2		23
#define SPI_C_CSPOL1		22
#define SPI_C_CSPOL0		21
#define SPI_C_RX		20
#define SPI_C_RXR		19
#define SPI_C_TXD		18
#define SPI_C_RXD		17
#define SPI_C_DONE		16
#define SPI_C_LEN		13
#define SPI_C_REN		12
#define SPI_C_ADCS		11
#define SPI_C_INTR		10
#define SPI_C_INTD		 9
#define SPI_C_DMAEN		 8
#define SPI_C_TA		 7
#define SPI_C_CSPOL		 6
#define SPI_C_CLEAR_RX		 5
#define SPI_C_CLEAR_TX		 4
#define SPI_C_CPOL		 3
#define SPI_C_CPHA		 2
#define SPI_C_CS1		 1
#define SPI_C_CS0		 0

/* delay*/
void wt(unsigned int delay )
{
	while(delay--)
		asm("mov r0, r0");	/* No-op */
}

void spi_pin_init(void)
{
	unsigned int var;
	/* Read current value of GPFSEL1- pin 10 & 11 */
	var = *gpioGPFSEL1;
	var &=~((7<<0) | (7<<3)); // set as input pins GPIO = 000
       	var |= ((4<<0) | (4<<3)); // set to alt function ALT0, GPIO = 000
	/* Write back updated value */
	*gpioGPFSEL1 =var;
	
	/* Read current value of GPFSEL1- pin 7,8 & 9 */
	var = *gpioGPFSEL0;
	var &=~((7<<21) | (7<<24) | (7<<27)); // set as input pins GPIO = 000
       	var |= ((4<<21) | (4<<24) | (4<<27)); // set to alt function ALT0, GPIO = 000
	/* Write back updated value */
	*gpioGPFSEL0 =var;
}
void spi_begin(void)
{	
	unsigned int var;
	spi_pin_init();
        
	*SPI0_CONTROL = 0;
	
	// Clear TX and RX fifos
	*SPI0_CONTROL |= (1 << SPI_C_CLEAR_RX) | (1 << SPI_C_CLEAR_TX);
	
	var = *SPI0_CONTROL;
 	// set data mode = 0 
	var &= ~((1 << SPI_C_CPOL) | (1 << SPI_C_CPHA));	
	
	//set to CS0 and Chip Select Polarity=HIGH
	var &= ~((1 << SPI_C_CS0) | (1 << SPI_C_CS1));
	var |= (1 << SPI_C_CSPOL0);

	/* Write back updated value */
	*SPI0_CONTROL = var;
        //set clock to 2 MHz
	*SPI0_CLK = 16;    
}

/*
unsigned int spi_transfer(unsigned char value)
{
	unsigned int var = 0;
	unsigned int ret = 0;
	
	var = *SPI0_CONTROL;
	var|= (1 << SPI_C_CLEAR_RX) | (1<<SPI_C_CLEAR_TX);// Clear TX and RX fifos
	// Set TA = 1
	var|= (1 << SPI_C_TA);
	// Write back updated value
	*SPI0_CONTROL =var;
	
	// Maybe wait for TXD
	var = *SPI0_CONTROL;
	while (!(var & (1 << SPI_C_TXD))) { var = *SPI0_CONTROL; }  

	// Write to TX FIFO
	*SPI0_FIFO = value;
	
	// Wait for DONE to be set
	var = *SPI0_CONTROL;
	while (!(var & (1 << SPI_C_DONE))) { var = *SPI0_CONTROL; }

	// Read RX FIFO
	ret = *SPI0_FIFO;

    // Set TA = 0, 
    var = *SPI0_CONTROL;
	var &= ~(1 << SPI_C_TA);
	
	// Write back updated value
	*SPI0_CONTROL =var;
	
    return ret;
}
*/

void spi_transfer(unsigned char* buffer, unsigned int length)
{
	unsigned int var = 0;
	unsigned int ret = 0;
	unsigned int i;
	
	var = *SPI0_CONTROL;
	var|= (1 << SPI_C_CLEAR_RX) | (1<<SPI_C_CLEAR_TX);// Clear TX and RX fifos
	// Set TA = 1
	var|= (1 << SPI_C_TA);
	// Write back updated value
	*SPI0_CONTROL =var;
	
	for (i = 0; i < length; i++) {
		// Maybe wait for TXD
		//var = *SPI0_CONTROL;
		//while (!(var & (1 << SPI_C_TXD))) { var = *SPI0_CONTROL; }  

		// Write to TX FIFO
		*SPI0_FIFO = buffer[i];
		
		// Wait for DONE to be set
		//var = *SPI0_CONTROL;
		while (!(*SPI0_CONTROL & (1 << SPI_C_DONE))) {}

		// Read RX FIFO
		//buffer[i] = *SPI0_FIFO;
	}
	
    // Set TA = 0, 
    //var = *SPI0_CONTROL;
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
	// 1 us delay
	1 + 1 + 1;
	*SPI0_CONTROL |= (1 << SPI_C_TA);
	wt(1000000);
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
	
	// Write back updated value
	//*SPI0_CONTROL =var;
}

void spi_end(void)
{
 	// Set all the SPI0 pins back to input
	unsigned int var;
	/* Read current value of GPFSEL1- pin 10 & 11 */
	var = *gpioGPFSEL1;
	var &=~((7<<0) | (7<<3)); // set as input pins GPIO = 000
       	/* Write back updated value */
	*gpioGPFSEL1 =var;
	
	/* Read current value of GPFSEL1- pin 7,8 & 9 */
	var = *gpioGPFSEL0;
	var &=~((7<<21) | (7<<24) | (7<<27)); // set as input pins GPIO = 000
    /* Write back updated value */
	*gpioGPFSEL1 =var;


}