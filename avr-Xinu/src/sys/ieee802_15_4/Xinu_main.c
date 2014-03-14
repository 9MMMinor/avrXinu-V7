//
//  Xinu_main.c
//  802_15_4_Mib
//
//  Created by Michael M Minor on 11/27/13.
//  Copyright 2013.
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//



/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <frame802154.h>
#include <avr/io.h>
#include <registermap.h>
#include <radio.h>
#include <macSymbolCounter.h>
#include <stdlib.h>
#include <rfr2_radioDriver.h>
#include <avr/pgmspace.h>


#include <mark.h>
#include <mem.h>
#include <ports.h>
#include <sleep.h>

int MLME_GET_request(int, int);
void pauseMicroSeconds(void *, uint32_t);
INTPROC printNow(void *);
PROCESS frameInput(int, int *);
PROCESS frameOutput(int, int *);

/*------------------------------------------------------------------------
 * Test frame headers and structures and mib functions
 *------------------------------------------------------------------------
 */

frame_t myFrame;
frame802154_t frameStructure;
union {
	uint8_t b[2];
	uint16_t v;
} value;

uint32_t time1, time2, time3;

int
main( void )
{
	octet_t *payload;
	radio_status_t status = 0;
	uint32_t value1, value2;
	int message;
	char errorMessage[40];
	
	/// Enter code here
	printf("Hello World\n");
	printf("Manufacture's Id = 0x%x\n", MAN_id0.id);
	sleep(1);
	
	macSymbolCounterInit();
	radioTimerEventInit();
	
	MLME_GET_request(0x40,0);
	sleep(1);
	
	
	status = radio_init();
	if (status != RADIO_SUCCESS)	{
		printf("Can't initialize: %s\n", radioStatusString(status));
	}
	
	status = radio_enter_sleep_mode();
	if (status == RADIO_SUCCESS)	{
		printf("Enter Sleep Mode\n");
	}
	else	{
		printf("Can't enter sleep mode. Status: 0x%02x\n", status);
	}
	sleep(5);
	
	if ( (status = radio_leave_sleep_mode()) == RADIO_SUCCESS)	{
		printf("Enter TRX_OFF mode\n");
	}
	else	{
		printf("Can't enter sleep mode. Status: 0x%02x\n", status);
	}
	
	if ( (status = radio_set_trx_state(CMD_RX_ON)) == RADIO_SUCCESS)	{
		printf("Enter RX_ON state\n");
	}
	else	{
		strncpy_P(errorMessage, radioStatusString(status), 40);
		printf("Status error: %d %s\n",status,errorMessage);
		printf("Operating channel = %d\n", radio_get_operating_channel());
	}
	
	sleep(1);
	
	value1 = macSymbolCounterRead();
	/* Set a timer to expire in 10000 symbol counter units */
	tmset(timerPortID, (void *)1, 1000000, &printNow);
	/* Receive message from port, timePort */
	preceive(timerPortID);	/* blocks until time event */
	value2 = macSymbolCounterRead();
	kprintf("Time port delay = %ld\n", value2-value1);
	
	sleep(1);
	
	tmset(timerPortID, (void *)1, 500000, NULL);
	tmset(timerPortID, (void *)2, 1000000, NULL);
	value1 = tmleft(timerPortID, (void *)1);
	value2 = tmleft(timerPortID, (void *)2);
	kprintf("Time left with message 1 = %ld\n", value1);
	kprintf("Time left with message 2 = %ld\n", value2);
//	kprintf("messages in queue = %d\n", pcount(timerPortID));
	message = (int)preceive(timerPortID);	/* blocks until time event */
	value1 = macSymbolCounterRead();
	kprintf("Message %d at %ld\n", message, value1);
//	kprintf("messages in queue = %d\n", pcount(timerPortID));
	message = (int)preceive(timerPortID);	/* blocks until time event */
	value1 = macSymbolCounterRead();
	kprintf("Message %d at %ld\n", message, value1);
	
	/* start the network input/output daemon process */
#ifdef RADIO_OUTPUT_TEST
	resume(
		   create(frameOutput, 600, 100, "frameOutput", 1, getpid())
		   );
#endif
	
#ifdef RADIO_INPUT_TEST
	resume(
		   create(frameInput, 600, 100, "frameInput", 1, getpid())
		   );
#endif
	
	for (;;)	{
		sleep(60);
		time1 = macSymbolCounterRead();				// 62500 counts per second
		pauseMicroSeconds((void *)3, 1000000L);
		time3 = macSymbolCounterRead();
//		kprintf("delta12=%ld delta23=%ld delta13=%ld\n",time2-time1, time3-time2, time3-time1);
	}
	return 0;
}


//void pauseMicroSeconds(void *message, uint32_t usec)
//{
//	uint32_t time = usec/16;	/* convert to symbol times */
	
//	tmset(timerPortID, message, time, &printNow);
//	preceive(timerPortID);	/* blocks until time event */
//}

INTPROC printNow(void *message)
{
	
	time2 = macSymbolCounterRead();
//	kprintf("printNow: port=%d msg=%d time=%ld\n",timerPortID,message,time1);
	
}

