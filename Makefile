MMCU = atmega32u4
FREQ = 16000000UL
XTAL = 16000000UL
FLASH_BAUD = 1200
# Frequencies in Hz
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = lib/%.h hibiscus/hibiscus.h hedera/hedera.h
OBJ = obj/hibiscus.o obj/hedera.o

PORT=/dev/ttyACM0

.PHONY: clean all flash

obj/hibiscus.o: ./hibiscus/hibiscus.c $(DEPS)
	$(CC) $(CFLAGS) -DF_OSC=$(XTAL) -DFLASH_BAUD=$(FLASH_BAUD)U -o $@ -c $<

obj/hedera.o: ./hedera/hedera.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: ./lib/%.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.o: ./main.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.elf: $(OBJ) ./obj/main.o 
	$(CC) $(CFLAGS) -o $@ $^
	
file.hex: ./obj/main.elf
	avr-objcopy -O ihex $< $@

all: file.hex
	avr-size -G ./obj/main.elf

flash: all
	stty -F $(PORT) $(FLASH_BAUD)
	sleep 1;
	avrdude -p atmega32u4 -c avr109 -P $(PORT) -U flash:w:file.hex:i

clean:
	rm ./obj/*
	rm file.hex
