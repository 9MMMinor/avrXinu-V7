#
#  Makefile for the avr-Xinu user:
#	802_15_4_Mib
#  Created by Michael M Minor on 11/27/13.
#
XINU_PROJECT = 802_15_4_Mib

all: $(XINU_PROJECT)

#SYSCOMPS = src/sys/sys src/sys/tty src/sys/net src/sys/shell src/sys/rf
SYSCOMPS = src/sys/sys src/sys/tty

#############################################################################
# Edit to change default Directory                                          #
#############################################################################
XINU_DIRECTORY = $(HOME)/XinuV7

AVR_XINUDIR = $(XINU_DIRECTORY)/avr-Xinu
XINU = $(AVR_XINUDIR)

CFLAGS += -D__avrXinu__

MAIN_SRC = mib.c frame802154.c rfr2_radioDriver.c macSymbolCounter.c radioStatusString.c radio.c \
			setTrxState.c
#(1)MAIN_SRC += Xinu_main.c frameInput.c frameOutput.c
#(2)MAIN_SRC += test_frameIO.c frameIO.c
#(3)MAIN_SRC += ProducerConsumer_frameIO.c frameIO.c
#(4)
MAIN_SRC += wirelessping.c frameIO.c
	
INCLUDE =	

include $(XINU)/Makefile.include
	 