MMCU = atmega32u4
# Frequencies in Hz
FREQ = 8000000UL
FLASH_BAUD = 1200
PORT = /dev/ttyACM0
PORT_WINDOWS = COM5
PORT_FLASH_WINDOWS = COM6

CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc

OBJ := hibiscus.o hyacinthus.o hydrangea.o hedera.o
SRC := $(patsubst ./src/%.c,./obj/%.o,$(filter-out ./src/main.c,$(wildcard ./src/*.c)))

.PHONY: clean all flash

obj/%.o: ./lib/%/*.c ./lib/%/*.h
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: ./src/%.c ./src/*.h
	$(CC) $(CFLAGS) $(patsubst %,-I%,$(wildcard ./lib/*)) -o $@ -c $<

obj/main.elf: ./obj/main.o $(SRC) $(patsubst %,./obj/%,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^
	
file.hex: ./obj/main.elf
	avr-objcopy -O ihex $< $@

all: file.hex
	avr-size -G ./obj/main.elf

flash: all
	stty -F $(PORT) $(FLASH_BAUD)
	sleep 1;
	avrdude -p atmega32u4 -c avr109 -P $(PORT) -U flash:w:file.hex:i

windows_pathset:
	set PATH=%PATH%;..\AVR-GCC\bin\;..\avrdude-x86\;

flashw: windows_pathset all
	@echo off
	mode $(PORT_WINDOWS) 1200,N,8,1
	timeout /t 1
	@echo on
	avrdude -p atmega32u4 -c avr109 -P $(PORT_FLASH_WINDOWS) -b 115200 -U flash:w:.\file.hex:i

	@echo off
	timeout /t 5

clean:
	rm ./obj/*
	rm file.hex
