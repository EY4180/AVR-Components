DEVICE     = ATmega1284
CLOCK      = 20000000

L_FUSE = 0b11110111
H_FUSE = 0b10011001
E_FUSE = 0xFF
PROGRAMMER = -e -v -p $(DEVICE) -c usbasp -P usb -D 

######################################################################
######################################################################

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.cpp.o:
	$(COMPILE) -c $< -o $@

flash:	all
	$(AVRDUDE) -u -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) -U lfuse:w:$(L_FUSE):m -U hfuse:w:$(H_FUSE):m

clean:
	del main.hex main.elf main.o

# file targets:
main.elf: main.o
	$(COMPILE) -o main.elf main.o

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size -d main.elf

disasm:	main.elf
	avr-objdump -d -S main.elf