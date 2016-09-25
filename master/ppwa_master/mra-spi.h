#ifndef MRA_SPI_H
#define MRA_SPI_H

#define SPI_C_LEN_LONG		25
#define SPI_C_DMA_LEN		24
#define SPI_C_CSPOL2		23
#define SPI_C_CSPOL1		22
#define SPI_C_CSPOL0		21
#define SPI_C_RX			20
#define SPI_C_RXR			19
#define SPI_C_TXD			18
#define SPI_C_RXD			17
#define SPI_C_DONE			16
#define SPI_C_LEN			13
#define SPI_C_REN			12
#define SPI_C_ADCS			11
#define SPI_C_INTR			10
#define SPI_C_INTD		 	9
#define SPI_C_DMAEN		 	8
#define SPI_C_TA		 	7
#define SPI_C_CSPOL		 	6
#define SPI_C_CLEAR_RX	 	5
#define SPI_C_CLEAR_TX	 	4
#define SPI_C_CPOL		 	3
#define SPI_C_CPHA		 	2
#define SPI_C_CS1		 	1
#define SPI_C_CS0		 	0

#define RET_SUCCESS			0
#define RET_IN_PROGRESS		1
#define RET_FAILED			2
#define RET_DEBUG			3
#define RET_FATAL_ERROR		4

#define EXIT_SUCCESS		0
#define EXIT_UNLOCK_ERR		1
#define EXIT_ERASE_ERR		2
#define EXIT_PROG_ERR		3
#define EXIT_VERIFY_ERR		4
#define EXIT_FATAL_ERR		5

#define BUFFER_SIZE			24
#define LARGE_BUFFER_SIZE	3000

#define RAM_ADDR			0x6000
#define FLASH_ADDR 			0x8000
#define WORD_LENGTH 		0x40
#define PAGE_LENGTH 		0x400
#define PAGE_COUNT 			31
#define CHECKSUM_LENGTH 	4

void SpiBegin(void);
void SpiEnd(void);
void SpiTransfer(unsigned char* buffer, unsigned int length);

unsigned int SetAddr(unsigned int addr);
unsigned int CmdReq(unsigned char cmd, unsigned int len, unsigned char* pars);
unsigned int Read(unsigned int len);
unsigned int Write(unsigned int len, unsigned char* data);
unsigned int GetStatus();

void SysReset();

unsigned int BootResetAndUnlock(void);
unsigned int BLFlashMassErase(void);
unsigned int BLFlashPageProg(unsigned int ram_addr, unsigned int flash_addr);
unsigned int BLFlashVerify(unsigned int byte_count);

unsigned int DIGetChipInfo();
unsigned int DIGetDeviceInfo();

void SetDebug(unsigned char debug_on);
unsigned char GetDebug(void);

#endif