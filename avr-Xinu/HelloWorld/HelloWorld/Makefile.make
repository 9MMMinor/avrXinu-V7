# -*- makefile -*-

#  Makefile for the avr-Xinu project:
#	HelloWorld
#  Created by Michael Minor on 4/5/15.

XINU_PROJECT = HelloWorld
#EDIT give the project a name
#
PROJECT := $(XINU_PROJECT)

#EDIT change bin directory if not specifically in $PATH
#
CHANGE_PATH_TO_BINTOOLS =

#
#            DMBS Build System
#     Released into the public domain.
#
#  dean [at] fourwalledcubicle [dot] com
#        www.fourwalledcubicle.com
#

# Run "make help" for target help.

#EDIT remove comment '#' if not using avrXinu makefile (avrXinu.make)
#
#MCU          = atmega256rfr2
#ARCH         = AVR8
#F_CPU        = 16000000
#OPTIMIZATION = s
#TARGET       = Template
#SRC          = $(XINU_PROJECT).c
#CC_FLAGS     =
#LD_FLAGS     =
#
# ToolChain in use
USE_CROSSPACK_AVR = yes
USE_BREW_HOMEBREW = no
USE_MACPORTS = no
LIBCPATH = /usr/local/CrossPack-AVR

#EDIT change the default Xinu Directory
#
#XINU_DIRECTORY := /Users/Xinu
XINU_DIRECTORY := /Users/mmm/XinuV7
AVR_XINUDIR := $(XINU_DIRECTORY)/avr-Xinu
HOME := $(AVR_XINUDIR)
XINU := $(HOME)
DMBS_PATH := $(HOME)
USE_DEBUG = no

#EDIT SYSCOMPS is a list of Xinu directories to include source files from Makerules
#     SYSCOMPS is sometimes defined in {Xinu}/platform/Makefile.{TARGET}
#     but it should be here.
#
SYSCOMPS := src/sys/sys src/sys/tty

#EDIT list of source files in this project directory/path
#
MAIN_SRC := Xinu_main.c

#EDIT Build flags (These all get added to (+=) later
INCLUDE :=
CFLAGS := -D__avrXinu__
LDFLAGS :=
#LDFLAGS := -Wl,-u,vfprintf -lprintf_flt -lm

# Default target
all: $(XINU_PROJECT)

# Include DMBS build script makefiles
DMBS_PATH   ?= ../DMBS
include $(DMBS_PATH)/core.make
include $(DMBS_PATH)/avrXinu.make
#include $(DMBS_PATH)/gcc.make
#include $(DMBS_PATH)/cppcheck.make
include $(DMBS_PATH)/doxygen.make
#include $(DMBS_PATH)/dfu.make
#include $(DMBS_PATH)/hid.make
include $(DMBS_PATH)/avrdude.make
#include $(DMBS_PATH)/atprogram.make
