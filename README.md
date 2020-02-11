# i2c_uart

Simple converter from I2C to UART.
Can be used for print out logs when the UART is used for other purposes.

General features:
- I2C slave address is 0x22
- I2C maximal boudrate is 100kbit
- console speed is 230400 boud

Important notice:
 Because of a very low resources, this converter uses a clock stretching
 every received byte when it's sent via UART.

Using:
 write logs to address 0x22 as I2C packets
 I2C proto: (START)(0x22+W)(DATA....DATA)(STOP)

 ATtiny13 pins:
 - pin 2 I2C-CLK
 - pin 3 I2C-SDA
 - pin 6 UART-TX
 - pin 7 LED (active low)

The file avrdude.conf is a modified version for my own FTDI programmer board.

Makefile and UART code is overtaken from (and based on) Lukasz Marcin Podkalicki project.
See:
	https://blog.podkalicki.com/100-projects-on-attiny13/

TODO: watchdog support

