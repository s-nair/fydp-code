#include "mra-spi.h"
#include "rpi-gpio.h"
#include "rpi-systimer.h"
#include "tfp-printf.h"

/* GPIO base */
volatile unsigned int* gpio = (unsigned int*)GPIO_BASE;

/** Main function - we'll never return from here */
void kernel_main( unsigned int r0, unsigned int r1, unsigned int atags )
{	
	unsigned int debug = 0xAAAAAAAA;
	unsigned char* debug_str;
	unsigned int ret;
	
	tfp_sprintf(debug_str, "0x%08X", debug);
	
	// Set LED 47 to output (DEBUG)
	gpio[LED_GPFSEL] |= ( 1 << LED_GPFBIT );
	
	// Turn off wait states
	SetDebug(1);
	
	SpiBegin();
	
	while(1){
		
		// Start signal
		gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(2000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(2000000);
		
		ret = BootResetAndUnlock();
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		SysReset();
		
		gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		ret = BLFlashMassErase();
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		ret = BLFlashPageProg(debug, debug);
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		ret = BLFlashVerify(debug);
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		ret = SetAddr(debug);
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
		
		ret = Write(( debug & 0xF ), debug_str);
		
		if (ret == RET_DEBUG) { gpio[LED_GPSET] = ( 1 << LED_GPIO_BIT ); }
		RPI_WaitMicroSeconds(1000000);
		gpio[LED_GPCLR] = ( 1 << LED_GPIO_BIT );
		RPI_WaitMicroSeconds(1000000);
	}
	
	SpiEnd();
}