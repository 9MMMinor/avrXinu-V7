//
//  README
//  Text
//  ----------------------------------
//
//  stk500V2 boot file on ATZB-S1-256-3-0-C
//  Created by Michael Minor on 12/7/14
//
//	12/7/14 Arduino bootloader:
//	Downloaded Arduino for Mac (1.0.6)
//	Downloaded Arduino for Mac (1.5.8)
//		Arduino.app/Contents/Resources/Java/hardware/arduino/bootloaders/stk500v2
//		files have NOT been updated to contain resources for atmega256rfr2 and atmega2564rfr2
//	"Atmel AT07252: RFR2 Arduino Solutions" application note cites support (in Windows)."
//

		 The stk500 Bootloader code of {Xinu}/platform/zigbit/stk500v2Boot
			has been updated to support the ATmega256RFR2 device
		 Support for the 256rfr2Zigbit device has been added to the Makefile present in the
			same directory as shown below:
		############################################################
		#	Dec 7, 2014	<avr-Xinu> Adding 16 Mhz ATmega256rfr2 Extension
		256rfr2Zigbit: MCU = atmega256rfr2
		256rfr2Zigbit: F_CPU = 16000000
		256rfr2Zigbit: BOOTFUSES = -U hfuse:w:0x98:m -U lfuse:w:0xE6:m
		256rfr2Zigbit: BOOTLOADER_ADDRESS = 3E000
		256rfr2Zigbit: CFLAGS += -D_AVR_256RFR2ZIGBIT_ -DBAUDRATE=115200
		#256rfr2Zigbit: CFLAGS += -DUSE_USART1
		256rfr2Zigbit: begin gccversion sizebefore build sizeafter end
				cp $(TARGET).hex stk500boot_v2_Zigbit.hex

ISP programming ATZB-S1-256-3-0-C
Jumper Settings
---------------

With 3-5 inch female jumpers, place a wire between ISP6PIN header on your programmer to the ATZB J105 header:

		ISP6PIN(1)>-------------MISO--------------<J105-17
		ISP6PIN(2)>-------------VTG---------------<J105-20
		ISP6PIN(3)>-------------SCK---------------<J105-18
		ISP6PIN(4)>-------------MOSI--------------<J105-16
		ISP6PIN(5)>-------------RESET-------------<J105-07
		ISP6PIN(6)>-------------GND---------------<J105-19

First make the bootstrap program:

	mmm$ cd {Xinu-directory-path}/platform/zigbit/stk500v2Boot
	mmm$ make clean
	mmm$ make 256rfr2Zigbit

Secondly, connect the USB to the programmer which is jumpered as above, and load the bootstrap:

	mmm$ avrdude -p atmega256rfr2 -P /dev/tty.usbserial-FTSMQBJS -c stk500v2 -U flash:w:stk500boot.hex
	mmm$ avrdude -p atmega256rfr2 -P /dev/tty.usbserial-FTSMQBJS -c stk500v2 -U hfuse:w:0x9a:m -U lfuse:w:0xE2:m


Now connect an FTDI USB Serial Bridge to the ATZB J105 and/or J3 header:

		RXI >------------------------------------------------< J105-14 (USART1 - TX) or J3-6 (USART0 - TX)
		TX0 >------------------------------------------------< J105-13 (USART1 - RX) or J3-7 (USART0 - RX)
		                   0.01
		DTR >---------------||-------------------------------< J105-07 (RSTN)

	Optionally power-up with (3.3V) from the Serial Bridge

		3V3 >------------------------------------------------< J105-20
		GND >------------------------------------------------< J105-19

Now the bootloader which is in memory emulates an stk500:

	mmm$ cd ../../../HelloWorld
	mmm$ avrdude -p atmega256rfr2 -P /dev/tty.usbserial-AH013G4A -c stk500v2 -U flash:w:HelloWorld.hex

