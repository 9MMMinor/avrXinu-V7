//
//  optiboot
//  Created by Michael Minor on 1/14/14
//  Copyright © 2014
//

1) Edit makefile:

% vim Makefile
...
################## EDIT frequency ############################################
atmega1284: AVR_FREQ = 11059200L
################## EDIT end ##################################################
...
############# EDIT for local environment #######################
GCCROOT = /usr/local/bin/
#AVRDUDE_CONF =/usr/local/CrossPack-AVR/avrdude-5.10/avrdude.conf
AVRDUDE_CONF =
AVRDUDE_ROOT = /usr/local/bin/
############# EDIT end #########################################
...
wq!
...

2) Make .hex file for the bootloader:

% make atmega1284

3) Burn device with bootloader:

% make atmega1284_isp