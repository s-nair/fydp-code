#include <string.h>
#include <stdlib.h>

#include "rpi-gpio.h"
#include "rpi-systimer.h"

/** GPIO Register set */
volatile unsigned int* gpio = (unsigned int*)GPIO_BASE;

void LED_ON() {
	gpio[LED_GPSET] = (1 << LED_GPIO_BIT);
}

void LED_OFF() {
	gpio[LED_GPCLR] = (1 << LED_GPIO_BIT);
}

/** Main function - we'll never return from here */
void kernel_main( unsigned int r0, unsigned int r1, unsigned int atags )
{
    /* Set the LED GPIO pin to an output to drive the LED */
    gpio[LED_GPFSEL] |= ( 1 << LED_GPFBIT );

    /* Never exit as there is no OS to exit to! */
    while(1)
    {
        /* Light the LED */
        LED_ON();
		RPI_WaitMicroSeconds(1000000);

        /* Set the GPIO16 output low ( Turn OK LED on )*/
        LED_OFF();
		RPI_WaitMicroSeconds(1000000);
    }
}