MMCU = atmega32u4
FREQ = 16000000UL
XTAL = 16000000UL
FLASH_BAUD = 1200
# Frequencies in Hz
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = lib/%.h
OBJ = # obj/hyacinthus.o obj/hydrangea.o obj/helianthus.o

LIBS = $(patsubst %, -I%, $(wildcard ./lib/*))

PORT = /dev/ttyACM0

.PHONY: clean all flash

obj/hibiscus.o: ./lib/hibiscus/hibiscus.c 
	$(CC) $(CFLAGS) -DF_OSC=$(XTAL) -DFLASH_BAUD=$(FLASH_BAUD)UL -o $@ -c $<

obj/hyacinthus.o: ./lib/hyacinthus/hyacinthus.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/hydrangea.o: ./lib/hydrangea/hydrangea.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/helianthus.o: ./lib/helianthus/helianthus.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/hedera.o: ./lib/hedera/hedera.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: ./lib/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.o: ./main.c $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) -o $@ -c $<

obj/main.elf: ./obj/main.o ./obj/hibiscus.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	
file.hex: ./obj/main.elf
	avr-objcopy -O ihex $< $@

all: file.hex
	avr-size -G ./obj/main.elf

flash: all
	stty -F $(PORT) $(FLASH_BAUD)
	sleep 1;
	avrdude -p atmega32u4 -c avr109 -P $(PORT) -U flash:w:file.hex:i

test:
	echo $(LIBS)

clean:
	rm ./obj/*
	rm file.hex
