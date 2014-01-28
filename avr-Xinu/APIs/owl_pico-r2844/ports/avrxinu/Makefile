XINU_PROJECT = pclient

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

APP_PATH = ../..
WLP_API_PATH = $(APP_PATH)/wlp_api
OWL_PATH = $(APP_PATH)/owl

MAIN_SRC = \
	$(WLP_API_PATH)/avrXinu_wlp_api.c \
	$(WLP_API_PATH)/wlp_inet.c \
	$(OWL_PATH)/core/owl_shell.c \
	$(OWL_PATH)/core/owl_debug.c \
	$(OWL_PATH)/core/owl_err.c \
	$(OWL_PATH)/core/owl_timer.c \
	$(OWL_PATH)/ports/avrxinu/owl_driver.c \
	$(OWL_PATH)/ports/avrxinu/owl_rtc.c \
	$(OWL_PATH)/ports/avrxinu/owl_led.c \
	$(OWL_PATH)/ports/avrxinu/owl_board.c \
	$(OWL_PATH)/ports/avrxinu/owl_temp.c \
	$(APP_PATH)/wlp_shell.c \
	$(APP_PATH)/wlp_ttcp.c \
	$(APP_PATH)/wlp_ntp.c \
	$(APP_PATH)/wlp_ping.c \
	$(APP_PATH)/httpd/match.c \
	$(APP_PATH)/httpd/web_content.c \
	$(APP_PATH)/httpd/wlp_httpd.c \
	$(APP_PATH)/STAmain.c\
#	$(OWL_PATH)/ports/avrxinu/rf/access.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfalloc.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfclose.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfcntl.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfdump.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfgetc.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfinit.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfio.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfmkpac.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfopen.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfputc.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfread.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfseek.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfsend.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rfwrite.c\
#	$(OWL_PATH)/ports/avrxinu/rf/remove.c\
#	$(OWL_PATH)/ports/avrxinu/rf/rename.c

INCLUDE =\
	 -I$(APP_PATH) \
	 -I$(WLP_API_PATH) \
	 -I$(OWL_PATH)/ports/avrxinu
	

include $(XINU)/Makefile.include
	 
