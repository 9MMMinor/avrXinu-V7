//
//  README
//  Text
//  ----------------------------------
//
//  stk500V2 boot file on 256rfr2XplainedPro
//  Created by Michael Minor on 1/29/14
//

Jumper Settings
---------------

With 3-5 inch female jumpers, place a wire between ISP6PIN header on your programmer to the 256rfr2XplainPro EXT5 header:

		ISP6PIN(1)>-------------MISO--------------<EXT5(17)
		ISP6PIN(2)>-------------VTG---------------<EXT5(20)
		ISP6PIN(3)>-------------SCK---------------<EXT5(18)
		ISP6PIN(4)>-------------MOSI--------------<EXT5(16)
		ISP6PIN(5)>-------------RESET-------------<RSTN
		ISP6PIN(6)>-------------GND---------------<EXT5(19)
		
Cut traces on 256rfr2XplainPro between EDBG and 256rfr2 (all EDBG straps).
No need to cut UART0-RXD AND -TXD straps -- cut removes these signals from EXT2, EXT3, and EXT5.
DO NOT power up with USB cable connected to DEBUG USB. Power up programmer.


mmm$ make clean
---------------
mmm$ make 256rfr2XplainPro fuse
-------------------------------
	avrdude -p atmega256rfr2 -P /dev/tty.usbserial* -c stk500v2 -U hfuse:w:0x98:m -U lfuse:w:0xE6:m
mmm$ make program
-----------------
	avrdude -p atmega256rfr2 -P /dev/tty.usbserial* -c stk500v2 -U flash:w:stk500boot.hex