# -*- makefile -*-
#
#	Makefile script for the avr-Xinu project:
#	
#
#  configuration.make
#	Makefile to build a Xinu system library in the project sub-directory, LIBDIR.
#	The library is always 'libx.a.'
#	Generate 'conf.c,' 'conf.h,' and 'confisr.c' in the project directory.
#
#  
#
#  Created by Michael Minor on 11/20/14.
#  Copyright (c) 2014.
#
#
#            DMBS Build System
#     Released into the public domain.
#
#  dean [at] fourwalledcubicle [dot] com
#        www.fourwalledcubicle.com
#

DMBS_BUILD_MODULES         +=	CONF
DMBS_BUILD_TARGETS         +=	lib
DMBS_BUILD_MANDATORY_VARS  +=	TARGET # ARCH MCU SRC
DMBS_BUILD_OPTIONAL_VARS   +=	BOARD OPTIMIZATION C_STANDARD CPP_STANDARD F_CPU C_FLAGS \
								CPP_FLAGS ASM_FLAGS CC_FLAGS LD_FLAGS OBJDIR OBJECT_FILES \
								DEBUG_TYPE DEBUG_LEVEL LINKER_RELAXATIONS COMPILER_PATH
DMBS_BUILD_PROVIDED_VARS   +=
DMBS_BUILD_PROVIDED_MACROS +=

# -----------------------------------------------------------------------------
#               DMBS GCC Compiler Buildsystem Makefile Module.
# -----------------------------------------------------------------------------
# DESCRIPTION:
#   Provides a single target to build a C, C++ and/or Assembly Xinu library
#   with the AVR-GCC compiler.
# -----------------------------------------------------------------------------
# MAKETARGETS:
#
#    lib                       - Build and archive source files into a library
#    clean                     - Remove all project intermediary and binary
#                                output files
#    mostlyclean               - Remove intermediary output files, but
#                                preserve binaries
#
# MANDATORY PARAMETERS:
#
#    TARGET                    - Platform name
#    ARCH                      - Device architecture name
#    MCU                       - Microcontroller device model name
#    SRC                       - List of input source files (*.c, *.cpp, *.S)
#
# OPTIONAL PARAMETERS:
#
#    OPTIMIZATION              - Optimization level
#    C_STANDARD                - C Language Standard to use
#    CPP_STANDARD              - C++ Language Standard to use
#    F_CPU                     - Speed of the CPU, in Hz
#    C_FLAGS                   - Flags to pass to the C compiler only
#    CPP_FLAGS                 - Flags to pass to the C++ compiler only
#    ASM_FLAGS                 - Flags to pass to the assembler only
#    CC_FLAGS                  - Common flags to pass to the C/C++ compiler and
#                                assembler
#    LD_FLAGS                  - Flags to pass to the linker
#    LINKER_RELAXATIONS        - Enable or disable linker relaxations to
#                                decrease binary size (note: can cause link
#                                failures on systems with an unpatched binutils)
#    OBJDIR                    - Directory for the output object and dependency
#                                files; if equal to ".", the output files will
#                                be generated in the same folder as the sources
#    OBJECT_FILES              - Extra object files to link in to the binaries
#    DEBUG_FORMAT              - Format of the debugging information to
#                                generate in the compiled object files
#    DEBUG_LEVEL               - Level the debugging information to generate in
#                                the compiled object files
#    COMPILER_PATH             - Location of the GCC toolchain to use
#
# PROVIDED VARIABLES:
#
#    (None)
#
# PROVIDED MACROS:
#
#    (None)
#
# -----------------------------------------------------------------------------

SHELL = /bin/sh

#MACROS		make macros for testing variables and defines
#
ERROR_IF_UNSET   ?= $(if $(filter undefined, $(origin $(strip $(1)))), $(error Makefile $(strip $(1)) value not set))
ERROR_IF_EMPTY   ?= $(if $(strip $($(strip $(1)))), , $(error Makefile $(strip $(1)) option cannot be blank))
ERROR_IF_NONBOOL ?= $(if $(filter Y N, $($(strip $(1)))), , $(error Makefile $(strip $(1)) option must be Y or N))

# Default values of optionally user-supplied variables
COMPILER_PATH      ?=
OPTIMIZATION       ?= s
F_CPU              ?=
C_STANDARD         ?= gnu99
CPP_STANDARD       ?= gnu++98
C_FLAGS            ?=
CPP_FLAGS          ?=
ASM_FLAGS          ?=
CC_FLAGS           ?=
OBJDIR             ?= .
OBJECT_FILES       ?=
DEBUG_FORMAT       ?= dwarf-2
DEBUG_LEVEL        ?= 2
LINKER_RELAXATIONS ?= Y

# Makefile debug	
pdump:
	@echo
	@echo PLATFORM = $(PLATFORM)
	@echo "target Makefile = $(target_makefile)"
	@echo TARGET = $(TARGET)

ifndef XINU
	${error XINU not defined! You must specify (in the caller) where Xinu resides}
endif

ifeq ($(TARGET),)
	-include Makefile.target
	ifeq ($(TARGET),)
		${info TARGET not defined, using target 'stk500'}
		TARGET=stk500
	else
		${info using saved target '${TARGET}'}
	endif
endif

# Sanity check user supplied values
########$(foreach MANDATORY_VAR, $(DMBS_BUILD_MANDATORY_VARS), $(call ERROR_IF_UNSET, $(MANDATORY_VAR)))
#$(call ERROR_IF_EMPTY, MCU)
#$(call ERROR_IF_EMPTY, TARGET)

$(call ERROR_IF_EMPTY, XINU_PROJECT)

#$(call ERROR_IF_EMPTY, ARCH)
#$(call ERROR_IF_EMPTY, OPTIMIZATION)
#$(call ERROR_IF_EMPTY, C_STANDARD)
#$(call ERROR_IF_EMPTY, CPP_STANDARD)
#$(call ERROR_IF_EMPTY, OBJDIR)
#$(call ERROR_IF_EMPTY, DEBUG_FORMAT)
#$(call ERROR_IF_EMPTY, DEBUG_LEVEL)
#$(call ERROR_IF_NONBOOL, LINKER_RELAXATIONS)

ARCH = AVR8
# Determine the utility prefix to use for the selected architecture
ifeq ($(ARCH), AVR8)
	CROSS        := $(COMPILER_PATH)avr
else ifeq ($(ARCH), XMEGA)
	CROSS        := $(COMPILER_PATH)avr
else ifeq ($(ARCH), UC3)
	CROSS        := $(COMPILER_PATH)avr32
else
	$(error Unsupported architecture "$(ARCH)")
endif

# Output Messages
MSG_INFO_MESSAGE := ' [INFO]    :'
MSG_COMPILE_CMD  := ' [GCC]     :'
MSG_ASSEMBLE_CMD := ' [GAS]     :'
MSG_NM_CMD       := ' [NM]      :'
MSG_REMOVE_CMD   := ' [RM]      :'
MSG_LINK_CMD     := ' [LNK]     :'
MSG_ARCHIVE_CMD  := ' [AR]      :'
MSG_SIZE_CMD     := ' [SIZE]    :'
MSG_OBJCPY_CMD   := ' [OBJCPY]  :'
MSG_OBJDMP_CMD   := ' [OBJDMP]  :'


#XINU_CONFIGURE	path to Xinu Configure App
#
XINU_CONFIGURE = $(XINU)/cpu/avr/config/config

#CONFIGURATION	output files from command-line program, 'config.'
#
CONFIGURATION = conf.c confisr.c conf.h

#CONFIG		path to the $(CONFIGURATION) files
# 6/21/13 keep the configuration with the project
#
CONFIG    = .

#OBJECTDIR		directory in PROJECT path for system and Xinu library.
#
OBJECTDIR = obj_$(TARGET)

#TARGET_MAKEFILE	include Makefile.$(TARGET) to get platform specific varables: Fuses, clock frequency, etc.
#
TARGET_MAKEFILE := $(wildcard $(XINU)/platform/$(TARGET)/Makefile.$(TARGET) ${foreach TDIR, $(TARGETDIRS), $(TDIR)/$(TARGET)/Makefile.$(TARGET)})

# Check if the target makefile exists, and create the object directory if necessary.
ifeq ($(strip $(TARGET_MAKEFILE)),)
	${error The target platform "$(TARGET)" does not exist (maybe it was misspelled?)}
else
	ifeq (${wildcard $(OBJECTDIR)},)
		DUMMY := ${shell mkdir $(OBJECTDIR)}
		COPYSOURCES = yes
	endif
	ifneq (1, ${words $(TARGET_MAKEFILE)})
		${error More than one TARGET Makefile found: $(TARGET_MAKEFILE)}
	endif
	include $(TARGET_MAKEFILE)
endif

USE_MAKEFILE_VARS = $(origin USE_XCCONFIG)

LIB_ARC  = ${LIBDIR}/libx.a

SYSCOMPS += src/lib/libxc
SYSCOMPS += platform/$(TARGET)
COMPS     = ${SYSCOMPS}

vpath = %.c $(COMPS)

# Include expected files, each should add its part to the compile source
# Makerules makefiles need $(TOPDIR) for their root
TOPDIR = $(XINU)
COMP_SRC :=
include ${COMPS:%=$(TOPDIR)/%/Makerules}

COMP_OBJ = $(patsubst %.S,%.o,$(filter %.S,$(COMP_SRC))) \
$(patsubst %.c,%.o,$(filter %.c,$(COMP_SRC)))

XLIB_SRC = $(notdir $(COMP_SRC))
# And just the object file name
XLIB_OBJ =  $(notdir $(COMP_OBJ))
XSRC=$(join $(addsuffix $(OBJECTDIR)/, $(dir $(XLIB_SRC))), $(notdir $(XLIB_SRC)))
XOBJ	= $(patsubst %.S,%.o,$(filter %.S,$(XSRC))) \
$(patsubst %.c,%.o,$(filter %.c,$(XSRC)))

ifeq ($(COPYSOURCES),yes)
DOCOPYSOURCES := ${shell cp -p $(COMP_SRC) $(OBJECTDIR)}
endif


# Path to conf.c and confisr.c files defining system devices
CONFC	=	$(CONFDIR)/conf.c
CONFISR	=	$(CONFDIR)/confisr.c
# Path to the conf.h file defining system devices, drivers, and other system defines
CONFH	=	$(CONFDIR)/conf.h

INCLUDE  += -I$(CONFDIR) -I$(INCDIR)

# C compilation flags
CFLAGS +=  -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(TARGET_ARCH) $(INCLUDE) $(DFLAG)


# Configure Xinu from 'avr-Configuration' file
$(CONFIGURATION):	avr-Configuration $(TARGET_MAKEFILE)
	@echo
	@echo $(MSG_CONFIG)
	rm -f version vn
	$(XINU_CONFIGURE) avr-Configuration
# Configuration support if 'USE_CONF_H = yes'
# append configuration data to "conf.h"
	@sh $(XINU)/mkvers.sh
	@VERS=`cat version`
	@echo >> conf.h
	@echo >> conf.h
	@echo "Appending conf.h for $(PLATFORM)"
	@echo "// Xinu platform: $(PLATFORM)" >> conf.h
	@echo "// Xinu conf.h header file" >> conf.h
	@echo "// MCU: $(TARGET_ARCH)" >> conf.h
	@echo "// F_CPU: $(CLOCK)" >> conf.h
	@echo >> conf.h
	@$(foreach v, \
	$(sort $(filter-out USE_CONF_H, $(filter \
		ATTACH_% \
		ENABLE_% \
		ENTER_% \
		FIFO_% \
		GCC_% \
		I2C_% \
		LED_% \
		UART_% \
		USE_% \
		WATCHDOG_%, $(.VARIABLES)))), \
	if [[ "$($(v))" == "y" || "$($(v))" == "yes" ]] ; then \
	echo "#define $(v)" >> conf.h ; \
	elif [[ -n "$($(v))" && "$($(v))" != "n" && "$($(v))" != "no" ]] ; then \
	echo "#define $(v) $($(v))" >> conf.h ; fi ;)
#	@echo "#define VERSION \""`cat version`"\"" >> conf.h

## Create an archive for this configuration
#$(LIB_ARC):	$(XOBJ)
#	@echo
#	@echo $(MSG_NEWLIBRARY)
#	rm -f $(LIB_ARC)
#	$(AR) rc $(LIB_ARC) $(XOBJ)
#	$(RANLIB) $(LIB_ARC)





