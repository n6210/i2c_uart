/**
 * Copyright (c) 2020, Taddy G. <fotonix@pm.me>
 * I2C ti UART converter
 */

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "uart.h"

#define I2C_SLAVE_ADDR	(0x22 << 1)

#define SCL 			_BV(PB3)
#define SDA 			_BV(PB4)
#define MASK_SDA		_BV(PCINT4)

#define R_SCL			(PINB & SCL)
#define R_SDA			(PINB & SDA)
#define R_BOTH			(PINB & (SDA|SCL))

#define LED				_BV(PB2)
#define LED_L()			PORTB &= ~LED;
#define LED_H()			PORTB |= LED;

#define BUS_FREE_TIME	5 // in usec

#define nop() do { __asm__ __volatile__ ("nop"); } while (0)
#define irq_en() do { GIFR |= _BV(PCIF); nop(); sei(); } while (0)

enum {
	SEQ_BUS_FREE = 0,
	SEQ_START,
	SEQ_DATA,
	SEQ_STOP
};

// Internal I2C status
volatile int8_t status = 0;

void __inline i2c_clk_keep(void)
{
	DDRB |= SCL; // SCL out
	PORTB &= ~SCL; // SCL=0
	nop(); // for sync
}

void __inline i2c_clk_free(void)
{
	DDRB &= ~SCL; // SCL in
	nop(); // for sync
}

void __inline i2c_ack(void)
{
	DDRB |= SDA;   // SDA as out
	PORTB &= ~SDA; // Set ACK (SDA=0)
	nop();         // For synchronization
	while (!R_SCL); // Wait for SCL=1
	while (R_SCL); // Wait for SCL=0
	DDRB &= ~SDA;  // SDA as in
	nop();         // For synchronization
}

void __inline i2c_detect_addr(void)
{
	uint8_t register bshift = 7;
	uint8_t i;
	uint8_t d = 0;

	for (i = 0; i < 8; i++) {
		while (!R_SCL); // Wait for SCL=1
		// Get SDA state
		if (R_SDA) {
			d |= 1 << bshift;
			while (R_SCL); // Wait for SCL=0
		} else {
			while (1) {
				uint8_t register x = PINB;
				if ((x & SCL) == 0) // SCL = 0
						break;
				if (x & SDA) { // SDA 0->1 stop
					status = SEQ_STOP;
					return;
				}
			}
		}
		bshift--;
	}

	if (d == I2C_SLAVE_ADDR) {
		status = SEQ_DATA;
		i2c_ack(); // ACK on our addr
	} else {
		status = SEQ_BUS_FREE;
		irq_en();
	}

	wdt_reset();
}

uint8_t __inline i2c_get_byte(void)
{
	uint8_t bshift = 7;
	uint8_t d = 0;
	uint8_t i;

	cli();
	for (i = 0; i < 8; i++) {
		while (!R_SCL); // Wait for SCL=1
		// Get SDA state
		if (R_SDA) {
			d |= 1 << bshift;
			while (R_SCL); // Wait for SCL=0
		} else {
			while (1) {
				uint8_t register x = PINB;
				if ((x & SCL) == 0) 
						break;
				if (x & SDA) {
					status = SEQ_STOP;
					return 0;
				}
			}
		}
		bshift--;
	}

	i2c_ack();
	wdt_reset();
	irq_en();

	return d;
}

void i2c_wait_for_start(void)
{
	uint8_t register cnt = BUS_FREE_TIME;

	LED_H();
	DDRB &= ~(SCL|SDA); // SDA|SCL in
	PORTB &= ~(SCL|SDA); // Hiz

	wdt_reset();

	// Wait for SCL=1 & SDA=1 stable for BUS_FREE_TIME
	while (cnt) {
		if (R_BOTH != (SCL|SDA))
			cnt = BUS_FREE_TIME;
		else
			cnt--;

		_delay_us(1);
	}
	status = SEQ_BUS_FREE;

	irq_en();

	// Wait for detecting START sequence
	while (status != SEQ_START) {
		wdt_reset();
	}
	cli();

	// Wait for SLC=0 SDA=0
	while (R_BOTH) { 
		wdt_reset();
	}
}

ISR(PCINT0_vect)
{
	uint8_t register pin;

	wdt_reset();

	pin = PINB;
	if (pin & SCL) {
		if ((pin & SDA) == 0) {
			status = SEQ_START;
			LED_L();
		} else {
			status = SEQ_STOP;
			LED_H();
		}
	}
}

ISR(WDT_vect)
{
	uart_puts("\nI2C-UART (WDT reset)\n");
}

int main(void)
{
	uint8_t register byte;

	PCMSK = MASK_SDA;
	GIMSK |= _BV(PCIE);
	GIFR |= _BV(PCIF);
	DDRB |= LED;

	uart_setup(); // Setup UART Tx pin as out

	wdt_enable(WDTO_15MS); // Set prescaler to 15ms
	WDTCR |= _BV(WDTIE); // Enable WD irq

	cli();

	while (1) {
		i2c_wait_for_start();
		i2c_detect_addr();

		if (status == SEQ_DATA) {
			while (1) {
				byte = i2c_get_byte();
				if (status == SEQ_STOP) {
					cli();
					break;
				} else {
					i2c_clk_keep();
					uart_putc(byte);
					i2c_clk_free();
				}
			}
		}
	}
}
