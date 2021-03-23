DEVICE     = atmega48p
CLOCK      = 16000000
PROGRAMMER = -e -v -p $(DEVICE) -c arduino -P COM3 -b 19200 -D 
OBJECTS    = main.o

######################################################################
######################################################################

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -std=c99 -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

clean:
	del main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size -d main.hex

disasm:	main.elf
	avr-objdump -d -S main.elf