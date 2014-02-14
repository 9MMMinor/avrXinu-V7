#
#  Makefile for the avr-Xinu user:
#	ieee802_15_4
#  Created by Michael M Minor on 11/27/13.
#
XINU_PROJECT = ieee802_15_4

all: $(XINU_PROJECT)

#SYSCOMPS = src/sys/sys src/sys/tty src/sys/net src/sys/shell src/sys/rf
SYSCOMPS = src/sys/sys src/sys/tty

#############################################################################
# Edit to change default Directory                                          #
#############################################################################
XINU_DIRECTORY = /Users/mmm/avrXinu-V7

AVR_XINUDIR = $(XINU_DIRECTORY)/avr-Xinu
HOME = $(AVR_XINUDIR)
XINU = $(HOME)

CFLAGS += -D__avrXinu__

MAIN_SRC = Xinu_main.c mib.c frame802154.c rfr2_radioDriver.c macSymbolCounter.c radioStatusString.c radio.c \
			frameInput.c setTrxState.c frameOutput.c
	
INCLUDE =	

include $(XINU)/Makefile.include
	 