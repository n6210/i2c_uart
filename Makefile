# --
# Copyright (c) 2017, Lukasz Marcin Podkalicki <lukasz@podkalicki.com>
# --

MCU=attiny13
FUSE_L=0x7A
FUSE_H=0xFF
F_CPU=9600000
CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
SIZE=avr-size
AVRDUDE=avrdude
CFLAGS=-std=c99 -Wall -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET=main
DEV=/dev/ttyUSB1

SRCS=main.c uart.c

all:
	${CC} ${CFLAGS} -o ${TARGET}.o ${SRCS} 
	${LD} -o ${TARGET}.elf ${TARGET}.o
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.o ${TARGET}.hex
	${SIZE} -C --mcu=${MCU} ${TARGET}.elf

flash:
	${AVRDUDE} -p ${MCU} -P ${DEV} -c dasa2ftdi -V -U flash:w:${TARGET}.hex:i

fuse:
	$(AVRDUDE) -p ${MCU} -P ${DEV} -c dasa2ftdi -U hfuse:w:${FUSE_H}:m -U lfuse:w:${FUSE_L}:m

clean:
	rm -f *.c~ *.o *.elf *.hex
