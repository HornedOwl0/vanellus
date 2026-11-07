MMCU := atmega32u4
# Frequencies in Hz
FREQ := 16000000UL
FLASH_BAUD := 1200
PORT := /dev/ttyACM0
PORT_WINDOWS := COM5
PORT_FLASH_WINDOWS := COM6

CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc

LIB := hibiscus.o hyacinthus.o hydrangea.o hedera.o
SRC := $(notdir $(wildcard ./src/*/*.c) $(filter-out ./src/main.c,$(wildcard ./src/*.c)))

SRCOBJ := $(patsubst %.c,./obj/%.o,$(SRC))
LIBOBJ := $(patsubst %.o,./obj/%.o,$(LIB))

INCL := $(patsubst %,-I%,$(wildcard ./lib/*/) $(wildcard ./src/*/))

.PHONY: clean all flash

.SILENT:

obj/%.o: ./lib/%/*.c ./lib/%/*.h
	echo $(dir $<)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: ./src/%/*.c ./src/*.h
	echo $(dir $<)
	$(CC) $(CFLAGS) $(INCL) -o $@ -c $<

obj/%.o: ./src/%.c ./src/*.h
	echo $<
	$(CC) $(CFLAGS) $(INCL) -o $@ -c $<

obj/main.elf: $(LIBOBJ) $(SRCOBJ) ./obj/main.o
	$(CC) $(CFLAGS) -o $@ $^
	
file.hex: ./obj/main.elf
	avr-objcopy -O ihex $< $@

all: file.hex
	avr-size -G ./obj/main.elf

flash: all
	stty -F $(PORT) $(FLASH_BAUD)
	-sleep 1;
	-avrdude -q -q -p atmega32u4 -c avr109 -P $(PORT) -U flash:w:file.hex:i

clean:
	-rm ./obj/*
	-rm file.hex

windows_pathset:
	$env:PATH+=";D:\arduino-avr-gcc\AVR-GCC\bin\;D:\arduino-avr-gcc\avrdude-x86\"

flashw: all 
	-mode $(PORT_WINDOWS) 1200,N,8,1
	-timeout /t 1
	-avrdude -q -q -p atmega32u4 -c avr109 -P $(PORT_FLASH_WINDOWS) -b 115200 -U flash:w:.\file.hex:i

cleanw:
	-del .\obj\* /f /q
	-del file.hex /f /q
