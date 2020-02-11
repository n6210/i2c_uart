# attiny13 i2c_uart

Simple converter from I2C slave to UART built using ATtiny13.
Can be used for print out logs when the UART is used for other purposes.

General features:
- ATtiny13 CPU @9.6MHz
- Full soft I2C slave (uses PCINT)
- Soft TX UART by Lukasz Podkalicki
- I2C slave address is 0x22 (as i2C to USB FTDI chips)
- I2C maximal boudrate is 100kbit
- UART speed is set to 230400 boud

Used ATtiny13 pins:
 - pin 1 RESET
 - pin 2 I2C-CLK
 - pin 3 I2C-SDA
 - pin 4 GND
 - pin 6 UART-TX
 - pin 7 LED (active low)
 - pin 8 +Vcc (3V3)

Using:
 write logs to address 0x22 as I2C packets
 I2C proto: (START)(0x22+W)(DATA....DATA)(STOP)

Important notice:
 Because of a very low resources, this converter uses a clock stretching
 every received byte when it's sent via UART.

The file avrdude.conf is a modified version for my own FTDI programmer board.

Makefile and UART code is overtaken from (and based on) Lukasz Marcin Podkalicki project.
See:
	https://blog.podkalicki.com/100-projects-on-attiny13/

TODO: watchdog support

