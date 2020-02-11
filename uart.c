/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * The ASM code is based on Ralph Doncaster's project (https://github.com/nerdralph/nerdralph/tree/master/avr/libs/bbuart)
 * Software UART for ATtiny13
 */

#include <avr/interrupt.h>
#include "uart.h"

void uart_setup(void)
{
	DDRB |= _BV(UART_TX);
	PORTB |= _BV(UART_TX);
}

void uart_putc(char c)
{
	uint8_t sreg;

	sreg = SREG;
	cli();
	__asm volatile(
		" cbi %[uart_port], %[uart_pin] \n\t" // start bit
		" in r0, %[uart_port] \n\t"
		" ldi r30, 3 \n\t" // stop bit + idle state
		" ldi r28, %[txdelay] \n\t"
		"TxLoop: \n\t"
		// 8 cycle loop + delay - total = 7 + 3*r22
		" mov r29, r28 \n\t"
		"TxDelay: \n\t"
		// delay (3 cycle * delayCount) - 1
		" dec r29 \n\t"
		" brne TxDelay \n\t"
		" bst %[ch], 0 \n\t"
		" bld r0, %[uart_pin] \n\t"
		" lsr r30 \n\t"
		" ror %[ch] \n\t"
		" out %[uart_port], r0 \n\t"
		" brne TxLoop \n\t"
		:
		: [uart_port] "I" (_SFR_IO_ADDR(PORTB)),
		[uart_pin] "I" (UART_TX),
		[txdelay] "I" (TXDELAY),
		[ch] "r" (c)
		: "r0","r28","r29","r30"
	);
	SREG = sreg;
	sei();
}

void uart_puts(const char *s)
{
     	while (*s) uart_putc(*(s++));
}
