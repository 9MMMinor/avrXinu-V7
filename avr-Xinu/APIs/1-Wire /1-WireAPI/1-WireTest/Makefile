#
#  Makefile for the avr-Xinu user:
#	1-WireAPI
#  Created by Michael Minor on 11/5/14.
#
XINU_PROJECT = 1-WireAPI

CHANGE_PATH_TO_BINTOOLS =

all: $(XINU_PROJECT)


#############################################################################
# Edit to change default Directory                                          #
#############################################################################
#XINU_DIRECTORY = /Users/Xinu
XINU_DIRECTORY = /Users/mmm/XinuV7

#AVR_XINUDIR = $(XINU_DIRECTORY)/avr-XinuExperimentalRoot
AVR_XINUDIR = $(XINU_DIRECTORY)/avr-Xinu
HOME = $(AVR_XINUDIR)
XINU = $(HOME)

SYSCOMPS = src/sys/sys src/sys/tty APIs/1-Wire/common APIs/1-Wire/I2Cbridge

CFLAGS += -D__avrXinu__

MAIN_SRC = Xinu_main.c $(XINU)/APIs/1-Wire/I2Cdriver/twi.c
	
INCLUDE =	

include $(XINU)/Makefile.include
	 
