# -*- makefile -*-
#
# avrXinu.make
#		resides in the directory $(XINU)

ifndef XINU
  ${error XINU not defined! You must specify (in the caller) where Xinu resides}
endif

ifeq ($(PLATFORM),)
  -include Makefile.platform
  ifeq ($(PLATFORM),)
    ${info PLATFORM not defined, using platform 'stk500'}
    PLATFORM=stk500
  else
    ${info using saved platform '$(PLATFORM)'}
  endif
endif

ifeq ($(DEFINES),)
  -include Makefile.$(PLATFORM).defines
  ifneq ($(DEFINES),)
    ${info using saved defines '$(DEFINES)'}
  endif
endif

ifndef HOST_OS
	HOST_OS := $(shell uname)
endif

usage:
	@echo "make MAKEPLATFORMS... [PLATFORM=(PLATFORM)] [saveplatform] [platforms]"

platforms:
	@ls -1 $(XINU)/platform $(PLATFORMDIRS) | grep -v CVS

saveplatform:
	-@rm -f Makefile.platform
	@echo "saving Makefile.platform"
	@echo >Makefile.platform "PLATFORM = $(PLATFORM)"

savedefines:
	-@rm -f Makefile.$(PLATFORM).defines
	@echo "saving Makefile.$(PLATFORM).defines"
	@echo >Makefile.$(PLATFORM).defines "DEFINES = $(DEFINES)"

OBJECTDIR = obj_$(PLATFORM)

LOWERCASE = -abcdefghijklmnopqrstuvwxyz
UPPERCASE = _ABCDEFGHIJKLMNOPQRSTUVWXYZ
PLATFORM_UPPERCASE := ${strip ${shell echo $(PLATFORM) | sed y!$(LOWERCASE)!$(UPPERCASE)!}}
CFLAGS += -DXINU=1 -DXINU_PLATFORM_$(PLATFORM_UPPERCASE)=1

# Just a left over scheme to synch with Xcode for Mac users
BUILD_CONFIGURATION = ../Makefile_Var.xcconfig


# Thanks for Help from WinAVR Makefile Template found
# here: http://winavr.sourceforge.net/WinAVR-user-manual.html
# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
# WinAVR Makefile Template written by Eric B. Weddington, Jˆrg Wunsch, et al.
#
# Released to the Public Domain
#
# Additional material for this makefile was written by:
# Peter Fleury
# Tim Henigan
# Colin O'Flynn
# Reiner Patommel
# Markus Pfaff
# Sander Pool
# Frederik Rouleau
#
#----------------------------------------------------------------------------


# include Makefile.$(PLATFORM) to get platform specific varables: Fuses, clock frequency, etc.
### Include target makefile

target_makefile := $(wildcard $(XINU)/platform/$(PLATFORM)/Makefile.$(PLATFORM) ${foreach TDIR, $(PLATFORMDIRS), $(TDIR)/$(PLATFORM)/Makefile.$(PLATFORM)})

# Check if the target makefile exists, and create the object directory if necessary.
ifeq ($(strip $(target_makefile)),)
	${error The target platform "$(PLATFORM)" does not exist (maybe it was misspelled?)}
else
	ifeq (${wildcard $(OBJECTDIR)},)
		DUMMY := ${shell mkdir $(OBJECTDIR)}
		COPYSOURCES = yes
	endif
	ifneq (1, ${words $(target_makefile)})
		${error More than one PLATFORM Makefile found: $(target_makefile)}
	endif
	include $(target_makefile)
endif

USE_MAKEFILE_VARS = $(origin USE_XCCONFIG)



#####################################################################
# 'make -p'  OR  'make -pn -f /dev/null'							#
#	TELLS you everything you never wanted to know about 'Makefile'	#
#####################################################################

AVR-XINUDIR = $(XINU)
MAIN_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(MAIN_SRC)))
#PORT = $(USB_SERIAL)

#PROGRAMMER = -c stk500v2 -P $(HOST_SERIAL)
#AVRDUDE_PROGRAMMER = arduino
#AVRDUDE_PROGRAMMER = stk500v2
#AVRDUDE_PROGRAMMER = avr109
#AVRDUDE_PART = $(PLATFORM_ARCH)
#AVRDUDE_PORT = $(PORT)
#AVRDUDE_UPLOAD_RATE = 57600
#AVRDUDE_WRITE_FLASH = -U flash:w:$(XINU_PROJECT).hex
#AVRDUDE_FLAGS = -F -p $(AVRDUDE_PART) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) \
#  -b $(AVRDUDE_UPLOAD_RATE)

# Program Settings
CC = avr-gcc
CPP = avr-g++
#LD = avr-ld
AR = avr-ar
RANLIB = avr-ranlib
SIZE = avr-size
# path to Xinu Configure App:
XINU_CONFIGURE = $(XINU)/cpu/avr/config/config
CONFIGURATION = conf.c confisr.c conf.h

LIBDIR    = ./$(OBJECTDIR)
CONFDIR   =	.
INCDIR	  =	$(XINU)/src/sys/h

# Path to the $(CONFIGURATION) files
# 6/21/13 keep the configuration with the project 
CONFIG    = .


LIB_ARC  = ${LIBDIR}/libx.a

SYSCOMPS += src/lib/libxc
SYSCOMPS += platform/$(PLATFORM)
COMPS     = ${SYSCOMPS}

vpath = %.c $(COMPS)

# Include expected files, each should add its part to the compile source
# Makerules makefiles need $(TOPDIR) for their root
TOPDIR = $(XINU)
COMP_SRC :=
include ${COMPS:%=${AVR-XINUDIR}/%/Makerules}

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

INCLUDE  += -I$(CONFDIR) -I$(INCDIR) -I$(LIBCPATH)/avr/include

# C compilation flags
CFLAGS +=  -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(PLATFORM_ARCH) $(INCLUDE) $(DFLAG)

# Assembler flags
ASFLAGS  = -x assembler-with-cpp

# Loader flags
LDFLAGS += -Wl,--wrap,malloc,--wrap,free,--wrap,realloc,--wrap,fdevopen $(EXTERNAL_RAM)

# -ffreestanding prevents optimization from replacing 'printf("no conversions\n");'
#    with 'puts("no conversions\n");' for example,
#    but who knows what else.
#    In one test -ffreestanding was shorter(!) than code generated without it
#    by more than the length of puts().
GCCFLAGS =  -ffreestanding -L$(OBJECTDIR)

MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_COMPILING_ARCHIVE = Compiling Archive:
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARCHIVE = Assembling Archive:
MSG_CLEANING = Cleaning project:
MSG_CONFIG = Configuring Xinu:
MSG_ARCHIVING = Archiving Xinu:
MSG_NEWLIBRARY = Creating a Xinu Library for this configuration:

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d
CFLAGS += $(GENDEPFLAGS)

$(XINU_PROJECT):	begin gccversion sizebefore build sizeafter finished end

build:	elf hex lss sym

elf:	$(XINU_PROJECT).elf
hex:	$(XINU_PROJECT).hex
lss:	$(XINU_PROJECT).lss
sym:	$(XINU_PROJECT).sym

#DEGUG make variables
# on the command line:
# $ make print-VAR
#	will echo the VAR, its origin, its flavor, and its value.
#
print-%:
	@echo '$*=$($*)'
	@echo '  origin = $(origin $*)'
	@echo '  flavor = $(flavor $*)'
	@echo '   value = $(value  $*)'

force:
	@echo

begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo

# Display size of file.
HEXSIZE = $(SIZE) --target=ihex $(XINU_PROJECT).hex
ELFSIZE = $(SIZE) -x -A $(XINU_PROJECT).elf
sizebefore:
	@if [ -f $(XINU_PROJECT).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(XINU_PROJECT).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi

# Display compiler version information.
gccversion :
	$(CC) --version

## Special rules for special files
#	Initialize Xinu gets a special VERSION
#
%initialize.o:	%initialize.c
	@echo
	@echo $(MSG_COMPILING) $(OBJECTDIR)/initialize.c
	@sh $(XINU)/mkvers.sh
	$(CC) -c -DVERSION=\""`cat version`"\" $(CFLAGS) $(GCCFLAGS) $< -o $@
#	$(CC) -c -Wall -O1 -DF_CPU=$(CLOCK) -mmcu=$(PLATFORM_ARCH) $(INCLUDE) $(DFLAG) $(GCCFLAGS) $< -o $@
	
# Compile: create object files from C++ source files.
%.o	: %.cpp
	@echo
	@echo $(MSG_COMPILING) $<
	$(CPP) -c $(ALL_CXXFLAGS) $< -o $@

# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(CFLAGS) $(GCCFLAGS) $< -o $@

# Compile: create object files from S assembler files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(CFLAGS) $(GCCFLAGS) $(ASFLAGS) $< -o $@

# Compile: create object files from s assembler files.
%.o	: %.s
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(CFLAGS) $(GCCFLAGS) $(ASFLAGS) $< -o $@

# if you use stk500v2 or other programmer, edit the command below appropriately:
flash: $(XINU_PROJECT).hex
	$(AVRDUDE) $(PROGRAMMER) -p $(AVRDUDE_PART) $(AVRDUDE_WRITE_FLASH)
	
# Program fuses
fuse:
	$(AVRDUDE) $(PROGRAMMER) -p $(AVRDUDE_PART) $(FUSES)

# Program fuses for using BOOT
bootfuse:
	$(AVRDUDE) $(PROGRAMMER) -p $(AVRDUDE_PART) $(BOOTFUSES)
	
# if you use arduino bootloader, edit the command below appropriately:
# Program the device.  
upload boot bootload: $(XINU_PROJECT).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)
	osascript $(XINU)/ScreenApplet.scpt
	
screen look:
	osascript $(XINU)/ScreenApplet.scpt
	
# Target: clean project.
clean: begin clean_tgt finished end

clean_tgt: 
	@echo
	@echo $(MSG_CLEANING)
	rm -f $(XINU_PROJECT).hex $(XINU_PROJECT).elf $(XINU_PROJECT).sym $(XINU_PROJECT).lss 
	rm -f *.o
	
cleandep: clean_tgt
	rm -f .deps
	touch .deps
	
cleanmost: cleandep
	rm -f $(XOBJ)
	rm -f $(MAIN_OBJ)
	rm -f $(LIB_ARC)
	rm -f vn version
	rm -f Makefile.bak
	
cleanall: cleanmost
	rm -f $(CONFIGURATION)
	rm -rf $(OBJECTDIR)

# Makefile debug	
vdump:
	@echo
	@echo PLATFORM = $(PLATFORM)
	@echo "target Makefile = $(target_makefile)"
	@echo "object Directory = $(OBJECTDIR)"
	@echo "SYSCOMPS = $(SYSCOMPS)"
	@echo
	@echo "XOBJ is $(XOBJ)"
	@echo
	@echo "MAIN_OBJ is $(MAIN_OBJ)"
	@echo
	@echo "archive is $(LIB_ARC):"
	@$(AR) -t $(LIB_ARC)

# Configure Xinu from 'avr-Configuration' file
$(CONFIGURATION):	avr-Configuration $(target_makefile)
	@echo
	@echo $(MSG_CONFIG)
	rm -f version vn
	$(XINU_CONFIGURE) avr-Configuration
# Configuration support if 'USE_CONF_H = yes'
# append configuration data to "conf.h"
	@sh $(XINU)/mkvers.sh
	@echo >> conf.h
	@echo >> conf.h
	@echo "Appending conf.h for $(PLATFORM)"
	@echo "// Xinu platform: $(XINU_PLATFORM)" >> conf.h
	@echo "// Xinu conf.h header file" >> conf.h
	@echo "// MCU: $(PLATFORM_ARCH)" >> conf.h
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
	@echo "//#define VERSION \""`cat version`"\"" >> conf.h

# Create an archive for this configuration
$(LIB_ARC):	$(XOBJ)
	@echo
	@echo $(MSG_NEWLIBRARY) 
	rm -f $(LIB_ARC)
	$(AR) rc $(LIB_ARC) $(XOBJ)
	$(RANLIB) $(LIB_ARC)

# Link
$(XINU_PROJECT).elf:	$(CONFIGURATION) $(LIB_ARC) $(OBJECTDIR)/initialize.o $(OBJECTDIR)/clkint.o $(MAIN_OBJ)
	@echo $(DEFS)
	@echo $(MSG_LINKING)
	$(CC) $(CFLAGS) $(GCCFLAGS) $(LDFLAGS) -o $(XINU_PROJECT).elf $(OBJECTDIR)/initialize.o $(OBJECTDIR)/clkint.o $(MAIN_OBJ) -lx -lc

# make Hex			
$(XINU_PROJECT).hex: $(XINU_PROJECT).elf
	rm -f $(XINU_PROJECT).hex
	avr-objcopy -j .text -j .data -O ihex $(XINU_PROJECT).elf $(XINU_PROJECT).hex

# Targets for code debugging and analysis:
disasm:	$(XINU_PROJECT).elf
	avr-objdump -h -t -d $(XINU_PROJECT).elf

# Create cross reference table
cref: $(XINU_PROJECT).elf
	$(CC) $(CFLAGS) $(GCCFLAGS) $(LDFLAGS) -Wl,--cref $(OBJECTDIR)/initialize.o $(OBJECTDIR)/clkint.o $(MAIN_OBJ) -lx -lc

# Create extended listing file from ELF output file.
$(XINU_PROJECT).lss: $(XINU_PROJECT).elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	avr-objdump -h -S $< > $@

# Create a symbol table from ELF output file. (e.g. make $(XINU_PROJECT).sym; cat $(XINU_PROJECT).sym)
$(XINU_PROJECT).sym: $(XINU_PROJECT).elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	avr-nm -n $< > $@

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# Listing of phony targets.
.PHONY : all begin build elf finished hex lss sym end flash boot upload bootload clean cleandep\
			clean_tgt cleanmost cleanall disasm cref vdump screen look force $(XINU_PROJECT)
