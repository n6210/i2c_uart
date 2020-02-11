/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * The ASM code is based on Ralph Doncaster's project (https://github.com/nerdralph/nerdralph/tree/master/avr/libs/bbuart)
 * Software UART for ATtiny13
 */

#ifndef	_UART_H_
#define	_UART_H_

#define UART_BAUDRATE		(115200 * 2)
#define	UART_TX_ENABLED		1 // Enable UART TX
#define UART_TX             PB1 // Use PB3 as TX pin

#define	TXDELAY         	(int)(((F_CPU / UART_BAUDRATE) - 7 + 1.5) / 3)

void uart_setup(void);
void uart_putc(char c);
void uart_puts(const char *s);

#endif	/* !_UART_H_ */
