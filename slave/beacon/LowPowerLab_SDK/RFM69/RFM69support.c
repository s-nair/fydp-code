//
//  RFM69support.c
//  
//
//  Created by Sharath Nair on 2016-11-08.
//
//

#include <stdio.h>
#include <string.h>
#include "nrf_drv_spi.h"
#include "RFM69support.h"
#include "SEGGER_RTT.h"

#define SPI_MISO_PIN				24
#define SPI_MOSI_PIN				23
#define SPI_SCK_PIN					22

#define SPI_INSTANCE  0 	/**< SPI instance index. */

static const nrf_drv_spi_t 	spi 			= NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  	/**< SPI instance. */
nrf_drv_spi_config_t 		spi_config 		= NRF_DRV_SPI_DEFAULT_CONFIG;
static volatile bool 		spi_xfer_done;  										/**< Flag used to indicate that SPI instance completed the transfer. */
static uint8_t       		m_rx_buf[1];    										/**< RX buffer. */


void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
}


/////////////////////
/* Arduino Library */
/////////////////////

uint8_t digitalRead(uint8_t pin) {
	uint8_t level;



	return level;
}

void digitalWrite(uint8_t pin, uint8_t level) {

}

void pinMode(uint8_t pin, uint8_t mode) {

}

uint32_t millis() {
	uint32_t millis;



	return millis;
}

void attachInterrupt(uint8_t irq_num, void (*f)(int), uint8_t trigger) {

}

void noInterrupts() {

}

void interrupts() {

}

void Serial_print(char *str) {
	SEGGER_RTT_WriteString(0, str);
}

void Serial_println(char *str) {
	uint32_t len = sizeof(str);
	char tmp[len + 3];

	tmp[len - 1] = '\n';
	tmp[len] = '\n';
	tmp[len + 1] = '\r';
	tmp[len + 2] = '\0';
	SEGGER_RTT_WriteString(0, tmp);
}

void Serial_print_hex(uint8_t val) {
	char *str;
	sprintf(str, "%X", val);

	SEGGER_RTT_WriteString(0, str);
}

void Serial_println_hex(uint8_t val) {
	char *str;
	sprintf(str, "%X", val);

	uint32_t len = sizeof(str);
	char tmp[len + 3];

	tmp[len - 1] = '\n';
	tmp[len] = '\n';
	tmp[len + 1] = '\r';
	tmp[len + 2] = '\0';
	SEGGER_RTT_WriteString(0, tmp);
}


/////////////////
/* SPI Library */
/////////////////

void SPI_begin() {
	spi_config.ss_pin    = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.miso_pin  = SPI_MISO_PIN;
	spi_config.mosi_pin  = SPI_MOSI_PIN;
	spi_config.sck_pin   = SPI_SCK_PIN;
	spi_config.frequency = NRF_DRV_SPI_FREQ_500K;

	nrf_drv_spi_init(&spi, &spi_config, spi_event_handler);
}

// this is mainly used for SPI transfer
uint8_t SPI_transfer_byte(uint8_t byte) {
	uint8_t m_length = sizeof(uint8_t);

	// Reset rx buffer and transfer done flag
	memset(m_rx_buf, 0, m_length);
	spi_xfer_done = false;

	nrf_drv_spi_transfer(&spi, &byte, m_length, m_rx_buf, m_length);

	while (!spi_xfer_done)
	{
		__WFE();
	}

	return *m_rx_buf;
}

// not really used, arduino supports it
void SPI_transfer(uint8_t * buffer, uint8_t size) {

}

void SPI_setDataMode(uint32_t mode) {
	spi_config.mode = mode;
}

void SPI_setBitOrder(uint32_t order) {
	spi_config.bit_order = order;
}

void SPI_setClockDivider(uint32_t div) {
	spi_config.frequency = div;
}

