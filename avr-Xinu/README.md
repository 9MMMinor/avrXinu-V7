avr-Xinu README

Installation Details
------------ -------

		Xinu$ cd {Xinu-directory}

This is file {Xinu-directory}/README.txt.  It contains instructions for installing the Xinu software and recompiling it.  These instructions must be followed:
	(a) after the distribution has been downloaded, or
	(b) after the Xinu directory has been moved.
	It is assumed that the AVR Libc and avr-GCC development tools have already been installed. If they have not, visit "http://www.obdev.at/products/crosspack/index.html" and download CrossPack for AVR.  Test a simple project. Be sure that the Environment variable, PATH, contains a path to your development tools:

		Xinu$ echo $PATH

Prepare your TARGET device (stk500 is the example):

		Xinu$ cd platform/stk500

	Edit "Makefile.stk500". You must edit the value of HOST_SERIAL. This is the path to the serial device which will connect to the programmer (STK500). In our case, "/dev/tty.usbserial-FTSMQBJS".  If your programmer is not a STK500, edit PROGRAMMER to reflect the change (-c programmer). Edit USB_SERIAL. This is the path to the serial line I/O from the target.

	Building a project references the source files listed in Makerules and compiling them.
	
Build a project (HelloWorld is the example):

		Xinu$ cd ../../HelloWorld
			
		or
		
		Xinu$ cd {XINU_DIECTORY}/avr-Xinu/HelloWorld

	Step 1:	Edit the Makefile in this directory. Edit the default Xinu path, XINU_DIRECTORY. From that the Makefile generates HOME and XINU (these are the path to the avr-Xinu root). SYSCOMPS are relative paths to the system sourcefiles. MAIN_SRC is the path to project source files. Make will climb around the tree and create objects of the source files. For every directory in "SYSCOMPS", make will add sources that are found in {SYSCOMP}/Makerules to the Makefile.

		Xinu$ cd {XINU_DIECTORY}/avr-Xinu/HelloWorld
		Xinu$ make

		This step rebuilds all necessary sources and libraries.  You do not need to be superuser; only directories under {Xinu-directory} are affected.

	Step 2:	Load the AVR microprocessor using AVRDUDE.
			
		Xinu$ make flash

	Step 3:	Start screen in a terminal window with its serial input connected to
			a 9600 baud serial port and to the RS232 Spare Port on the STK500.
			On Reset, Xinu will boot and print "hello world." and stop.
			
		Xinu$ /usr/bin/screen /dev/tty.usbserial-FTSMQBJS 9600
		Éreset
		Xinu Version AVR7.2.0 (Mar 6, 2010)   MCUCSR=2
		Heap: 0x1438 of length 11264
		16730 bytes Xinu code, SP=0x40ff
		clock enabled

		Hello, World!

		All user processes have completed.
		Exit screen (control-A control-\)


	Step 4:	You are ready to experiment with Xinu itself. Change to the {XINU} directory and copy HelloWorld to projects/MyProject. But before you copy, clean-up.
	
		Xinu$ make cleanall
		Xinu$ cd ../..
		Xinu$ cp -R HelloWorld projects/MyProject

		Change to your new MyProject directory, make it, clean it, and edit everything until you own it.

		Xinu$ cd cd projects/MyProject
		Xinu$ make
		Xinu$ make cleanall
		...edit file "avr-Configuration"
		...edit file "Xinu-main.c"
		...edit/create target ("make targets" lists existing targets)

		Xinu$ make
		Xinu$ make flash

Make will recompile the Xinu sources, rebuild file "Xinu_main.o", and rebind library "libx.a".  Make leaves the new versions in the project directory. Make flash uploads the target system using avrdude.  Other arguments can be used with make.  For details see the file "Makefile.include".  Two particularly useful ones are "disasm" that dumps the load image, and "cref" that outputs the cross reference file. Clean can be "cleanall", "cleanmost", "cleandep", or "clean".

If the contents of the directory XcodeTemplates is copied to ~/Library/Developer/Xcode/Templates,

		Xinu$ cp -R {XINU_DIECTORY}/avr-Xinu/ProjectTemplates/ ~/Library/Developer/Xcode/Templates

	you can build a project with Xcode. When you have the configuration/platform you will be using from Step 4, create a project directory in any dirctory you choose.  Create Xinu_main.c and other files if needed.
