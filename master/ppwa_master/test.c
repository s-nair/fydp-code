#define GPIO_BASE 	0x3F200000UL
#define GPFSEL4		4
#define FSEL47		21
#define GPSET1		8
#define GPCLR1		11
#define LED			15

volatile unsigned int* gpio;
volatile unsigned int* gpfs4;
volatile unsigned int* gpset1;
volatile unsigned int* gpclr1;
volatile unsigned int t;

int main (void) {
	
	gpio = (unsigned int*)GPIO_BASE;
	//gpfs4 = (unsigned int*)(gpio + (4 * GPFSEL4));
	//gpset1 = (unsigned int*)(gpio + (4 * GPSET1));
	//gpclr1 = (unsigned int*)(gpio + (4 * GPCLR1));
	
	gpio[GPFSEL4] |= (1 << FSEL47);
	
	while (1) {
		
		gpio[GPSET1] = (1 << LED);
		for (t = 0; t < 50000; t++)
			;
		
		gpio[GPCLR1] = (1 << LED);
		for (t = 0; t < 50000; t++)
			;
		
	}
	return 0;
}