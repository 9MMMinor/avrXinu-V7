# Name: Makefile
# Author: <insert your name here>
# Copyright: <insert your copyright message here>
# License: <insert your license reference here>

# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "stk500v2"
#                   default_serial = "avrdoper"
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

DEVICE     = atmega256rfr2
CLOCK      = 16000000

#----------------------------------------------------------------------------
# Edit host serial line device (Serial Port to programmer)
#----------------------------------------------------------------------------
HOST_SERIAL = /dev/tty.usbserial*


OBJECTS    = main.o ascdatetime.o mktime.o scanTime.o normalizeTm.o getUTC.o calendar_times.o
OBJECTS	+= day_of_epoch.o localtime.o gmtime.o SunRiseSunSet.o isleap.o day_of_year.o

FUSES      = -U hfuse:w:0x99:m -U lfuse:w:0xE6:m

# Atmega256rfr2 fuse bits used above (fuse bits for other devices are different!):
# Example for 16 MHz external oscillator
# Extended Fuse byte:
# 0xFE = 1 1 1 1   1 1 1 0 <-- BODLEVEL0 (Brown-Out Reset 1.8V)
#        ^ ^ ^ ^   ^ ^ ^------ BODLEVEL1
#        | | | |   | +-------- BODLEVEL2
#        | | | |   +---------- reserved
#        | | | +-------------- "
#        | | +---------------- "
#        | +------------------ "
#        +-------------------- "
# Fuse high byte:
# 0x99 = 1 0 0 1   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   +---------- EESAVE (set to 0 to preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (if set to 0, watchdog is always on)
#        | | +---------------- SPIEN (if set to 0, serial programming enabled)
#        | +------------------ JTAGEN (if set to 0, JTAG enabled)
#        +-------------------- OCDEN (if set to 0, enable On-Chip Debug)
# Fuse low byte:
# 0xE6 = 1 1 1 0   0 1 1 0
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (0010 default set to 0110 for 16MHz crystal)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ CKOUT (if 0 output the system clock on the CLKO pin)
#        +-------------------- CKDIV8 (initial divide clock by 8 if 0)
#
# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/

#  Fuses for STK500boot/Optiboot Bootloader
HFUSE = 0x98
LFUSE = 0xE6
EFUSE = 0xFE
UNLOCK_BITS = 0x3F
LOCK_BITS = 0x0F


# Tune the lines below only if you know what you are doing:

#AVRDUDE_PROGRAMMER = arduino
PROGRAMMER = -c stk500v2 -P $(HOST_SERIAL)
AVRDUDE_UPLOAD_RATE = 115200

AVRDUDE_FLAGS = -F -b $(AVRDUDE_UPLOAD_RATE)

# Program Settings
AVRDUDE	= avrdude
INCLUDE = -I/Users/mmm/XinuV7/avr-Xinu/src/sys/h
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $(INCLUDE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

# if you use stk500v2 or other programmer, edit the command below appropriately:
flash:	all
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(PROGRAMMER) -p $(DEVICE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install:
	cp time.h ../../sys/h
	cp date.h ../../sys/h

doxygen:
	doxygen

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
#	avr-size --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
