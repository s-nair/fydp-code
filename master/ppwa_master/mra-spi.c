#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mra-spi.h"
#include "rpi-gpio.h"
#include "rpi-systimer.h"
#include "tfp-printf.h"

/* GPIO PINS */
volatile unsigned int *gpioGPFSEL0 =  (unsigned int *) 0x3F200000;
volatile unsigned int *gpioGPFSEL1 =  (unsigned int *) 0x3F200004;
volatile unsigned int *gpioGPSET0  =  (unsigned int *) 0x3F20001C;
volatile unsigned int *gpioGPCLR0  =  (unsigned int *) 0x3F200028;
volatile unsigned int *gpioGPLEV0  =  (unsigned int *) 0x3F200034;

/* SPI0 ADDRESS */
volatile unsigned int *SPI0_CONTROL =  (unsigned int *) 0x3F204000;
volatile unsigned int *SPI0_FIFO    =  (unsigned int *) 0x3F204004;
volatile unsigned int *SPI0_CLK     =  (unsigned int *) 0x3F204008;
volatile unsigned int *SPI0_DLEN    =  (unsigned int *) 0x3F20400C;
volatile unsigned int *SPI0_LTOH    =  (unsigned int *) 0x3F204010;
volatile unsigned int *SPI0_DC      =  (unsigned int *) 0x3F204014;

volatile unsigned char debug = 0;

///////////////
/* Set Debug */
///////////////
void SetDebug(unsigned char debug_on)
{
	debug = debug_on;
}

///////////////
/* Get Debug */
///////////////
unsigned char GetDebug(void)
{
	return debug;
}

////////////////////////////
/* SPI Pin Initialization */
////////////////////////////
void spi_pin_init(void)
{
	unsigned int var;
	// Read current value of GPFSEL1 - pin 10 & 11
	var = *gpioGPFSEL1;
	var &=~((7<<0) | (7<<3)); 				// set as input pins GPIO = 000
    var |= ((4<<0) | (4<<3) | (1<<18)); 	// set to alt function ALT0, GPIO = 100 (GPIO 16 as output = 001)
	// Write back updated value
	*gpioGPFSEL1 =var;
	
	// Read current value of GPFSEL0 - pin 7,8 & 9
	var = *gpioGPFSEL0;
	var &=~((7<<21) | (7<<24) | (7<<27)); 	// set as input pins GPIO = 000
    var |= ((4<<21) | (4<<24) | (4<<27)); 	// set to alt function ALT0, GPIO = 000
	// Write back updated value
	*gpioGPFSEL0 =var;
}

////////////////////////
/* SPI Initialization */
////////////////////////
void SpiBegin(void)
{	
	unsigned int var;
	spi_pin_init();
	
	*SPI0_CONTROL = 0;
	
	// Clear TX and RX fifos
	*SPI0_CONTROL |= (1 << SPI_C_CLEAR_RX) | (1 << SPI_C_CLEAR_TX);
	
	var = *SPI0_CONTROL;
 	// Set data mode = 0 
	var &= ~((1 << SPI_C_CPOL) | (1 << SPI_C_CPHA));	
	
	// Set to CS0 and Chip Select Polarity=HIGH
	var &= ~((1 << SPI_C_CS0) | (1 << SPI_C_CS1));
	var |= (1 << SPI_C_CSPOL0);

	// Write back updated value
	*SPI0_CONTROL = var;
	switch (GetDebug()) {
		case (0):
			// Set clock to 15.625 MHz
			*SPI0_CLK = 16;
			break;
		default:
			// Set clock to 250 kHz (DEBUG)
			*SPI0_CLK = 1024;
			break;
	}
}

/////////////////////
/* SPI Pin Cleanup */
/////////////////////
void SpiEnd(void)
{
 	// Set all the SPI0 pins back to input
	unsigned int var;
	// Read current value of GPFSEL1 - pin 10 & 11
	var = *gpioGPFSEL1;
	var &=~((7<<0) | (7<<3)); // set as input pins GPIO = 000
    // Write back updated value
	*gpioGPFSEL1 =var;
	
	// Read current value of GPFSEL0 - pin 7,8 & 9
	var = *gpioGPFSEL0;
	var &=~((7<<21) | (7<<24) | (7<<27)); // set as input pins GPIO = 000
    // Write back updated value
	*gpioGPFSEL1 =var;
}

/////////////////////////////
/* Wait For CMD_RDY (MISO) */
// TODO: use separate GPIO pin to monitor MISO
/////////////////////////////
void WaitState()
{
	/*
	switch (GetDebug()) {
		case (0):
			// Wait until MISO goes high
			while (!(*gpioGPLEV0 & (1 << 16))) {}
			break;
		default:
			RPI_WaitMicroSeconds(10);
			break;
	}
	*/
	RPI_WaitMicroSeconds(10);
}

////////////////////
/* Spi Read|Write */
////////////////////
void SpiTransfer(unsigned char* buffer, unsigned int length)
{
	unsigned int var = 0;
	unsigned int i;
	
	var = *SPI0_CONTROL;
	// Clear TX and RX fifos
	var|= (1 << SPI_C_CLEAR_RX) | (1<<SPI_C_CLEAR_TX);
	// Set TA = 1
	var|= (1 << SPI_C_TA);
	// Write back updated value
	*SPI0_CONTROL =var;
	
	// Wait for MISO to go high
	WaitState();
	
	for (i = 0; i < length; i++) {
		// Wait for TXD
		while (!(*SPI0_CONTROL & (1 << SPI_C_TXD))) {}  

		// Write to TX FIFO
		*SPI0_FIFO = buffer[i];
		
		// Wait for DONE to be set
		while (!(*SPI0_CONTROL & (1 << SPI_C_DONE))) {}

		// Read RX FIFO
		buffer[i] = *SPI0_FIFO;
	}
	
    // Set TA = 0, 
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
}

//////////////
/* SET_ADDR */
//////////////
unsigned int SetAddr(unsigned int addr)
{
	unsigned char buffer[BUFFER_SIZE];
	unsigned int len;
	unsigned int sw;
	
	buffer[0] = (addr >> 8) & (0x7F);
	buffer[1] = (addr & 0xFF);
	len = 2;
	
	SpiTransfer(buffer, len);
	
	return (unsigned int)(( buffer[0] << 8 ) | buffer[1]);
}

/////////////
/* CMD_REQ */
/////////////
unsigned int CmdReq(unsigned char cmd, unsigned int len, unsigned char* pars)
{
	unsigned char buffer[BUFFER_SIZE];
	unsigned char tmp[2];
    char* endptr;
	unsigned int i;
	
	// Command
	buffer[0] = (3 << 6) | cmd;
	buffer[1] = (len & 0xFF);
	
	// Pars (each char in pars represents one nibble)
	for (i = 1; i < len + 1; i++) {
	    tmp[0] = pars[i << 1];
	    tmp[1] = pars[(i << 1) + 1];
		buffer[i + 1] = (unsigned char)strtoul(tmp, &endptr, 16);
	}
	
	SpiTransfer(buffer, len + 2);
	
	return (unsigned int)(( buffer[0] << 8 ) | buffer[1]);
}

///////////
/* READ */
// temp implementation, does not return data
///////////
unsigned int Read(unsigned int len)
{
	unsigned char buffer[LARGE_BUFFER_SIZE];
	unsigned int i;
	
	// Command + length
	buffer[0] = (9 << 4) | ( (len >> 8) & (0xF) );
	buffer[1] = (len & 0xFF);
	
	// send zeros while reading data 
	for (i = 2; i < len + 2; i++) {
		buffer[i] = 0;
	}
	
	SpiTransfer(buffer, len + 2);
	
	return (unsigned int)(( buffer[0] << 8 ) | buffer[1]);
}

///////////
/* WRITE */
///////////
unsigned int Write(unsigned int len, unsigned char* data)
{
	unsigned char buffer[LARGE_BUFFER_SIZE];
	unsigned int sw;
	unsigned char tmp[2];
    char* endptr;
	unsigned int i;
	
	// Command + length
	buffer[0] = (8 << 4) | ( (len >> 8) & (0xF) );
	buffer[1] = (len & 0xFF);
	
	// Data (each char in data represents one nibble)
	for (i = 1; i < len + 1; i++) {
	    tmp[0] = data[i << 1];
	    tmp[1] = data[(i << 1) + 1];
		buffer[i + 1] = (unsigned char)strtoul(tmp, &endptr, 16);
	}
	
	SpiTransfer(buffer, len + 2);
	
	return (unsigned int)(( buffer[0] << 8 ) | buffer[1]);
}

////////////////
/* GET_STATUS */
////////////////
unsigned int GetStatus()
{
	unsigned char buffer[BUFFER_SIZE];
	
	// Command + length
	buffer[0] = ((1 << 7) & 0xFF);
	buffer[1] = (0 & 0xFF);
	
	SpiTransfer(buffer, 2);
	
	return (unsigned int)(( buffer[0] << 8 ) | buffer[1]);
}

//////////////////////////////////
/* BOOT_RESET and BL_UNLOCK_SPI */
//////////////////////////////////
unsigned int BootResetAndUnlock()
{
	unsigned char buffer[BUFFER_SIZE];
	unsigned int length;
	unsigned char cmd;
	unsigned char* pars;
	unsigned int sw;
	
	unsigned int var;
	unsigned int i;
	
	buffer[0] = 0xB0;
	buffer[1] = 0x00;
	length = 2;
	
	var = *SPI0_CONTROL;
	// Clear TX and RX fifos
	var|= (1 << SPI_C_CLEAR_RX) | (1<<SPI_C_CLEAR_TX);
	// Set TA = 1
	var|= (1 << SPI_C_TA);
	// Write back updated value
	*SPI0_CONTROL =var;
	
	// Wait for 2 ms (bootup case)
	RPI_WaitMicroSeconds(2000);
	
	for (i = 0; i < length; i++) { 
		// Write to TX FIFO
		*SPI0_FIFO = buffer[i];
		
		// Wait for DONE to be set
		while (!(*SPI0_CONTROL & (1 << SPI_C_DONE))) {}
	}
	
    // Set TA = 0
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
	
	// 1 us delay
	1 + 1 + 1;
	
	// Set TA = 1
	*SPI0_CONTROL |= (1 << SPI_C_TA);
	
	// Unlock SPI
	cmd = 0x00;
	length = 4;
	pars = "0x2505B007";	// magic number
	
	sw = CmdReq(cmd, length, pars);
	
	// Check device response
	switch (sw) {
		// 0x8021: Waiting for the BL_UNLOCK_SPI() command
		case (0x8021):
			return RET_IN_PROGRESS;
		// 0x8020: Unlocking succeeded	
		case (0x8020):
			return RET_SUCCESS;
		// 0x0022: Unlocking failed	
		case (0x0022):
			return RET_FAILED;
		// 0xC025: DEBUG
		case (0xC025):
			return RET_DEBUG;
		// Unrecognized: Fatal error	
		default:
			return RET_FATAL_ERROR;
	}
}

///////////////
/* SYS_RESET */
///////////////
void SysReset()
{
	unsigned char buffer[BUFFER_SIZE];
	unsigned int length;
	unsigned int var;
	unsigned int i;
	
	buffer[0] = 0xBF;
	buffer[1] = 0xFF;
	length = 2;
	
	var = *SPI0_CONTROL;
	// Clear TX and RX fifos
	var|= (1 << SPI_C_CLEAR_RX) | (1<<SPI_C_CLEAR_TX);
	// Set TA = 1
	var|= (1 << SPI_C_TA);
	// Write back updated value
	*SPI0_CONTROL =var;
	
	// Wait for 2 ms (bootup case)
	RPI_WaitMicroSeconds(2000);
	
	for (i = 0; i < length; i++) { 
		// Write to TX FIFO
		*SPI0_FIFO = buffer[i];
		
		// Wait for DONE to be set
		while (!(*SPI0_CONTROL & (1 << SPI_C_DONE))) {}
	}
	
    // Set TA = 0
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
	
	// 1 us delay
	//1 + 1 + 1;
	RPI_WaitMicroSeconds( 1 );
	
	// Set TA = 1
	*SPI0_CONTROL |= (1 << SPI_C_TA);
	
	// Wait until MISO goes high
	WaitState();
	
	// Set TA = 0
	*SPI0_CONTROL &= ~(1 << SPI_C_TA);
}

/////////////////////////
/* BL_FLASH_MASS_ERASE */
/////////////////////////
unsigned int BLFlashMassErase()
{
	unsigned char cmd;
	unsigned int length;
	unsigned char* pars;
	unsigned int sw;
	
	// flash memory mass erase
	cmd = 0x03;
	length = 4;
	pars = "0x25051337";	// magic number
	
	sw = CmdReq(cmd, length, pars);
	
	// Check device response
	switch (sw) {
		// 0x0002: Mass erase in progress
		case (0x0002):
			return RET_IN_PROGRESS;
		// 0x8003: Mass erase completed successfully	
		case (0x8003):
			return RET_SUCCESS;
		// 0x8004: Command failed due to incorrect KEY parameter
		case (0x8004):
			return RET_FAILED;
		// 0xC325: DEBUG
		case (0xC325):
			return RET_DEBUG;
		// Unrecognized: Fatal error	
		default:
			return RET_FATAL_ERROR;
	}
}

////////////////////////
/* BL_FLASH_PAGE_PROG */
////////////////////////
unsigned int BLFlashPageProg(unsigned int ram_addr, unsigned int flash_addr)
{
	unsigned char cmd;
	unsigned int length;
	unsigned int dword_count;
	unsigned int key;
	unsigned char pars[BUFFER_SIZE];
	unsigned int sw;
	
	// program page in flash memory 
	cmd = 0x07;
	length = 10;
	dword_count = 0x0100;	// 1 kB page size
	key = 0x25051337;		// magic number
	
	tfp_sprintf(pars, "0x%04X%04X%04X%08X", ( ram_addr & 0xFFFF ), ( flash_addr & 0xFFFF ), dword_count, key);
	
	sw = CmdReq(cmd, length, pars);
	
	// Check device response
	switch (sw) {
		// 0x000A: Programming in progress
		case (0x000A):
			return RET_IN_PROGRESS;
		// 0x800B: Programming completed successfully	
		case (0x800B):
			return RET_SUCCESS;
		// 0x800C: Command failed due to incorrect KEY parameter
		case (0x800C):
			return RET_FAILED;
		// 0xC7AA: DEBUG
		case (0xC7AA):
			return RET_DEBUG;
		// Unrecognized: Fatal error	
		default:
			return RET_FATAL_ERROR;
	}
}

/////////////////////
/* BL_FLASH_VERIFY */
/////////////////////
unsigned int BLFlashVerify(unsigned int byte_count)
{
	unsigned char cmd;
	unsigned int length;
	unsigned int data_addr;
	unsigned char pars[BUFFER_SIZE];
	unsigned int sw;
	
	// program page in flash memory 
	cmd = 0x0F;
	length = 8;
	data_addr = 0x00008000;
	
	tfp_sprintf(pars, "0x%08X%08X", ( data_addr & 0xFFFFFFFF ), ( byte_count & 0xFFFFFFFF ));
	
	sw = CmdReq(cmd, length, pars);
	
	// Check device response
	switch (sw) {
		// 0x000D: Verification in progress
		case (0x000A):
			return RET_IN_PROGRESS;
		// 0x800E: Verification completed successfully (the image is intact)	
		case (0x800B):
			return RET_SUCCESS;
		// 0x800F: Command failed due to checksum mismatch
		case (0x800C):
			return RET_FAILED;
		// 0xCFAA: DEBUG
		case (0xCF00):
			return RET_DEBUG;
		// Unrecognized: Fatal error	
		default:
			return RET_FATAL_ERROR;
	}
}

/////////////////////////////////////////////////////////////
// Device Information Commands //
/////////////////////////////////////////////////////////////

/////////////////////
/* DI_GET_CHIP_INFO */
/////////////////////
unsigned int DIGetChipInfo()
{
	unsigned char cmd;
	unsigned int length;
	unsigned char* pars;
	unsigned int sw;
	
	// get chip info command
	cmd = 0x1F;
	length = 2;
	pars = "0x00B0";	// magic number
	
	CmdReq(cmd, length, pars);
	
	// read command
	length = 24;
	
	sw = Read(length);
}

/////////////////////
/* DI_GET_DEVICE_INFO */
/////////////////////
unsigned int DIGetDeviceInfo()
{
	unsigned char cmd;
	unsigned int length;
	unsigned char* pars;
	unsigned int sw;
	
	// get chip info command
	cmd = 0x1E;
	length = 0;
	pars = "";
	
	CmdReq(cmd, length, pars);
	
	// read command
	length = 12;
	
	sw = Read(length);
}

/////////////////////////////////////////////////////////////
// RF Test Commands //
/////////////////////////////////////////////////////////////

