MMCU = atmega32u4
FREQ = 16000000UL # in hertz (Hz)
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = lib/%.h
OBJ = obj/scheduler.o obj/UART.o 

.PHONY: clean all flash

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

flash: all
	avrdude -p m32u4 -c avr109 -P /dev/ttyACM0 -U flash:w:main.hex:i

clean: $(OBJ)
	rm $^
	rm main.elf
	rm main.hex
