MMCU = atmega32u4
FREQ = 16000000UL # in hertz (Hz)
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = hibiscus.h

.PHONY: clean all
hibiscus.o: hibiscus.c
	$(CC) $(CFLAGS) -o $@ -c $^

all: hibiscus.o

clean: $(OBJ)
	rm ./obj/main.elf
	rm file.hex
