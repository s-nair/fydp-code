//
//  RFM69support.h
//  
//
//  Created by Sharath Nair on 2016-11-08.
//
//

#ifndef RFM69support_h
#define RFM69support_h

#define LOW			0
#define HIGH		1

#define FALLING		0
#define RISING		1

#define INPUT		0
#define OUTPUT		1

#define SPI_INSTANCE  0 	/**< SPI instance index. */

/* Other Functions */
uint32_t current_ticks();
uint32_t millis_diff(uint32_t ticks_now, uint32_t ticks_old);

/* Arduino Library */
uint32_t digitalRead(uint32_t pin);
void digitalWrite(uint32_t pin, uint8_t level);
void pinMode(uint32_t pin, uint8_t mode);
void attachInterrupt(uint8_t irq_num, void (*f)(int), uint8_t trigger);
void noInterrupts();
void interrupts();
void Serial_print(char * str);
void Serial_println(char * str);
void Serial_print_hex(uint8_t val);
void Serial_println_hex(uint8_t val);

/* SPI Library */
void SPI_begin(uint8_t miso_pin, uint8_t mosi_pin, uint8_t sck_pin);
uint8_t SPI_transfer_byte(uint8_t byte);
void SPI_transfer(uint8_t * buffer, uint8_t size);
void SPI_setDataMode(uint32_t mode);
void SPI_setBitOrder(uint32_t order);
void SPI_setClockDivider(uint32_t div);


#endif /* RFM69support_h */
