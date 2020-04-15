CPU = 	#TINY13

ifeq ($(CPU),TINY13)
MCU =		attiny13
FUSE_L =	0x7A
FUSE_H =	0xFF
F_CPU =		9600000
else
MCU =		attiny85
F_CPU =		16500000
FUSE_L =	0xE1
FUSE_H =	0xDF
FUSE_EXT =	-U efuse:w:0xFF:m
endif

CC = 		avr-gcc
LD =		avr-ld
OBJCOPY =	avr-objcopy
SIZE = 		avr-size
AVRDUDE = 	avrdude
CFLAGS = 	-std=c99 -Wall -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET = 	main

DEV = 		/dev/ttyUSB2
ADPAR =		-P ${DEV} -c dasa2ftdi
ADPAR =		-c usbasp -B3

SRCS=main.c uart.c

all:
	${CC} ${CFLAGS} -o ${TARGET}.o ${SRCS} 
	${LD} -o ${TARGET}.elf ${TARGET}.o
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.o ${TARGET}.hex
	${SIZE} -C --mcu=${MCU} ${TARGET}.elf

flash:
	${AVRDUDE} -p ${MCU} ${ADPAR} -V -U flash:w:${TARGET}.hex:i -v

fuse:
	$(AVRDUDE) -p ${MCU} ${ADPAR} -U hfuse:w:${FUSE_H}:m -U lfuse:w:${FUSE_L}:m ${FUSE_EXT}

clean:
	rm -f *.c~ *.o *.elf *.hex
