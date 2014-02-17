#
#  Makefile for the avr-Xinu user:
#	HelloWorld
#  Created by Michael Minor on 1/14/14.
#
XINU_PROJECT = HelloWorld

CHANGE_PATH_TO_BINTOOLS =

all: $(XINU_PROJECT)


#############################################################################
# Edit to change default Directory                                          #
#############################################################################
XINU_DIRECTORY = $(HOME)/XinuV7

AVR_XINUDIR = $(XINU_DIRECTORY)/avr-Xinu
XINU = $(AVR_XINUDIR)

SYSCOMPS = src/sys/sys src/sys/tty

CFLAGS += -D__avrXinu__

MAIN_SRC = Xinu_main.c
	
INCLUDE =	

include $(XINU)/Makefile.include
	 
