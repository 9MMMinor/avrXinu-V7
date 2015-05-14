README.txt

Start a Terminal window and execute the following commands to build and download
an avr-Xinu target to a ATMEGA1284p plugged into STK500.

% make usage
% make PLATFORM=stk500 saveplatform
% make
% make avrdude
% make screen