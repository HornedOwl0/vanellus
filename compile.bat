:main
    @echo off

    set FREQ=1000000UL
    set XTAL=16000000UL

    @echo on

    ..\AVR-GCC\bin\avr-gcc -std=gnu23 -mmcu=atmega32u4 -Wall -Oz -DF_CPU=%FREQ% -DF_OSC=%XTAL% -o .\obj\hibiscus.o -c .\hibiscus\hibiscus.c

    ..\AVR-GCC\bin\avr-gcc -std=gnu23 -mmcu=atmega32u4 -Wall -Oz -DF_CPU=%FREQ% -o .\obj\main.o -c .\main.c
    ..\AVR-GCC\bin\avr-gcc -std=gnu23 -mmcu=atmega32u4 -Wall -Oz -DF_CPU=%FREQ% -o .\obj\main.elf .\obj\main.o .\obj\hibiscus.o
    ..\AVR-GCC\bin\avr-objcopy -O ihex .\obj\main.elf file.hex
    ..\AVR-GCC\bin\avr-size -G .\obj\main.elf

    @echo off

    set /p rst="port for resetting: "
    set /p flsh="port for flashing: "

    mode %rst% 1200,N,8,1
    timeout /t 1
    @echo on
    ..\avrdude-x86\avrdude -p atmega32u4 -c avr109 -P %flsh% -b 115200 -U flash:w:.\file.hex:i

    @echo off
        timeout /t 5