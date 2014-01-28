XINU_PROJECT = 1-Wire

all: $(XINU_PROJECT)


#############################################################################
# Edit to change default Directory                                          #
#############################################################################
XINU_DIRECTORY = /Users/mmm/XinuV7

AVR_XINUDIR = $(XINU_DIRECTORY)/avr-Xinu
HOME = $(AVR_XINUDIR)
XINU = $(HOME)


CFLAGS += -DBYTE_ORDER=LITTLE_ENDIAN -DOWLSH_CMDLEN=80
CFLAGS += -D__avrXinu__

SYSCOMPS = src/sys/sys src/sys/tty src/sys/shell

MAIN_SRC = Xinu_mainDS2482.c 24LC256.c
TWI_SRC	= twi.c DS2482_channel_select.c DS2482_detect.c DS2482_reset.c \
			DS2482_search_triplet.c DS2482_write_config.c OWBlock.c OWFamilySkipSetup.c \
			OWFirst.c OWLevel.c OWNext.c OWReadBitPower.c OWReadByte.c OWReset.c \
            OWSearch.c OWSpeed.c OWTargetSetup.c OWVerify.c OWWriteBit.c OWWriteByte.c OWWriteBytePower.c
MAIN_SRC += $(TWI_SRC)

INCLUDE =

include $(XINU)/Makefile.include
