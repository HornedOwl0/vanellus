MMCU = atmega32u4
FREQ = 16000000UL # in hertz (Hz)
CFLAGS = -std=gnu23 -mmcu=$(MMCU) -Wall -Oz -DF_CPU=$(FREQ)
CC = avr-gcc
DEPS = lib/%.h hibiscus/hibiscus.h
OBJ = 

PORT=/dev/ttyACM0

.PHONY: clean all flash

obj/hibiscus.o: ./hibiscus/hibiscus.c ./hibiscus/hibiscus.h
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: ./lib/%.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.o: ./main.c
	$(CC) $(CFLAGS) -o $@ -c $<

obj/main.elf: $(OBJ) ./obj/main.o ./obj/hibiscus.o
	$(CC) $(CFLAGS) -o $@ $^
	
file.hex: ./obj/main.elf
	avr-objcopy -O ihex $< $@

all: file.hex
	avr-size -G ./obj/main.elf

flash: all
	avrdude -p atmega32u4 -c avr109 -P /dev/ttyACM0 -U flash:w:file.hex:i

clean: $(OBJ)
	rm ./obj/hibiscus.o
	rm ./obj/main.elf
	rm file.hex
