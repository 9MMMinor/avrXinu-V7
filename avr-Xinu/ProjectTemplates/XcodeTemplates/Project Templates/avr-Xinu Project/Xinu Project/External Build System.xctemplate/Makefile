#
#  Makefile for the avr-Xinu user:
#	___PROJECTNAME___
#  Created by ___FULLUSERNAME___ on ___DATE___.
#
XINU_PROJECT = ___PROJECTNAME___

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

SYSCOMPS = src/sys/sys src/sys/tty

CFLAGS += -D__avrXinu__

MAIN_SRC = Xinu_main.c
	
INCLUDE =	

include $(XINU)/Makefile.include
	 
