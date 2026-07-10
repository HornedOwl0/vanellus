MMCU = atmega328p
FREQ = 16000000UL # in hertz (Hz)
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = lib/scheduler.h lib/UART.h lib/servo_timer1.h lib/utils.h
OBJ = obj/scheduler.o obj/UART.o 

.PHONY: clean all

obj/%.o: ./lib/%.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.o: ./main.c
	$(CC) $(CFLAGS) -o $@ -c $<

main.elf: $(OBJ) obj/main.o
	$(CC) $(CFLAGS) -o $@ $^
	
main.hex: main.elf
	avr-objcopy -O ihex $< $@

all: main.hex
	avr-size -G main.elf

clean: $(OBJ)
	rm $^
	rm main.elf
	rm main.hex