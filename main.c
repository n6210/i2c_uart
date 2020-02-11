/**
 * Copyright (c) 2020, Taddy G. <fotonix@pm.me>
 * I2C ti UART converter
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

#define I2C_SLAVE_ADDR	(0x22 << 1)

#define SCL 			_BV(PB3)
#define SDA 			_BV(PB4)
#define MASK_SDA		_BV(PCINT4)

#define R_SCL			(PINB & SCL)
#define R_SDA			(PINB & SDA)
#define R_BOTH			(PINB & (SDA|SCL))

#define TEST			_BV(PB2)
#define TEST_L			PORTB &= ~TEST;
#define TEST_H			PORTB |= TEST;

#define BUS_FREE_TIME	100 // in usec

#define nop() do { __asm__ __volatile__ ("nop"); } while (0)
#define irq_en() do { GIFR |= _BV(PCIF); nop(); sei(); } while (0)

enum {
	SEQ_BUS_FREE = 0,
	SEQ_START,
	SEQ_DATA,
	SEQ_STOP
};

volatile int8_t status = 0;

void __inline i2c_clk_keep(void)
{
	DDRB |= SCL;
	PORTB &= ~SCL;
	nop();
}

void __inline i2c_clk_free(void)
{
	DDRB &= ~SCL;
	PORTB &= ~SCL;
	nop();
}

void __inline i2c_ack(void)
{
	DDRB |= SDA; // SDA as out
	PORTB &= ~SDA; // Set ACK (SDA=0)
	nop();
	while (R_SCL == 0); // Wait for SCL=1
	while (R_SCL); // Wait for SCL=0
	DDRB &= ~SDA; // SDA as in
	PORTB &= ~SDA; // HiZ
	nop();
}

void __inline i2c_detect_addr(void)
{
	uint8_t register bshift = 7;
	uint8_t i;
	uint8_t d = 0;

TEST_L;
	for (i = 0; i < 8; i++) {
		while (R_SCL == 0); // Wait for SCL=1
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
		//TEST_H;
		irq_en();
	}
}

uint8_t __inline i2c_get_byte(void)
{
	uint8_t bshift = 7;
	uint8_t d = 0;
	uint8_t i;

	cli();
	for (i = 0; i < 8; i++) {
		while (R_SCL == 0); // Wait for SCL=1
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
	irq_en();

	return d;
}

void i2c_wait_for_start(void)
{
	uint8_t register cnt = BUS_FREE_TIME;

	TEST_H;

	DDRB &= ~(SCL|SDA); // SDA|SCL in
	PORTB |= (SCL|SDA); // Hiz

	// Wait for SCL=1 & SDA=1 stable for 100us
	while (cnt) {
		if (R_BOTH != (SCL|SDA))
			cnt = BUS_FREE_TIME;
		else
			cnt--;

		_delay_us(1);
	}
	status = SEQ_BUS_FREE;

	irq_en();

	while (status != SEQ_START);
	cli();

	while (R_BOTH); // Full bus ready SLC=0 SDA=0
}

ISR(PCINT0_vect)
{
	uint8_t register pin;

	pin = PINB;
	if (pin & SCL) {
		if ((pin & SDA) == 0) {
			status = SEQ_START;
			TEST_L;
		} else {
			status = SEQ_STOP;
			TEST_H;
		}
	}
}

int main(void)
{
	uint8_t register byte;

	PCMSK = MASK_SDA;
	GIMSK |= _BV(PCIE);
	GIFR |= _BV(PCIF);

	uart_setup();
	DDRB |= TEST;

	uart_puts("\nI2C to UART\n");
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
