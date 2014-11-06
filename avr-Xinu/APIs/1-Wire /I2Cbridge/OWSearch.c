//
//  OWSearch.c
//  TWIDriver
//
//  Created by Michael Minor on 1/9/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include <1-WireAPI.h>
#include <DS2482.h>
#include <twi.h>

//------------Copyright (C) 2008 Maxim Integrated Products --------------
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL MAXIM INTEGRATED PRODCUTS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Maxim Integrated Products
// shall not be used except as stated in the Maxim Integrated Products
// Branding Policy.
// ---------------------------------------------------------------------------
//
//  an3684.C - Application Note 3684
//

//--------------------------------------------------------------------------
// The 'OWSearch' function does a general search.  This function
// continues from the previous search state. The search state
// can be reset by using the 'OWFirst' function.
// This function contains one parameter 'alarm_only'.
// When 'alarm_only' is TRUE (1) the find alarm command
// 0xEC is sent instead of the normal search command 0xF0.
// Using the find alarm command 0xEC will limit the search to only
// 1-Wire devices that are in an 'alarm' state.
//
// Returns:   TRUE (1) : when a 1-Wire device was found and its
//                       Serial Number placed in the global ROM 
//            FALSE (0): when no new device was found.  Either the
//                       last search was the last device or there
//                       are no devices on the 1-Wire Net.
//
int OWSearch()
{
	int id_bit_number;
	int last_zero, rom_byte_number, search_result;
	int id_bit, cmp_id_bit;
	unsigned char rom_byte_mask, search_direction, status;
	
	// initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = FALSE;
	crc8 = 0;
	
	// if the last call was not the last one
	if (!LastDeviceFlag)
	{       
		// 1-Wire reset
		if (!OWReset())
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;
		}
		
		// issue the search command 
		OWWriteByte(0xF0);  
		
		// loop to do the search
		do
		{
			// if this discrepancy if before the Last Discrepancy
			// on a previous next then pick the same as last time
			if (id_bit_number < LastDiscrepancy)
			{
				if ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0)
					search_direction = 1;
				else
					search_direction = 0;
			}
			else
			{
				// if equal to last pick 1, if not then pick 0
				if (id_bit_number == LastDiscrepancy)
					search_direction = 1;
				else
					search_direction = 0;
			}
			
			// Perform a triple operation on the DS2482 which will perform 2 read bits and 1 write bit
			status = DS2482_search_triplet(search_direction);
			
			// check bit results in status byte
			id_bit = ((status & DS2482_STATUS_SBR) == DS2482_STATUS_SBR);
			cmp_id_bit = ((status & DS2482_STATUS_TSB) == DS2482_STATUS_TSB);
			search_direction = ((status & DS2482_STATUS_DIR) == DS2482_STATUS_DIR) ? 0x01 : 0x00;
			
			// check for no devices on 1-Wire
			if ((id_bit) && (cmp_id_bit))
				break;
			else
			{
				if ((!id_bit) && (!cmp_id_bit) && (search_direction == 0))
				{
					last_zero = id_bit_number;
					
					// check for Last discrepancy in family
					if (last_zero < 9)
						LastFamilyDiscrepancy = last_zero;
				}
				
				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					ROM_NO[rom_byte_number] &= (uint8_t)~rom_byte_mask;
				
				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;
				
				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					calc_crc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // (do-while) loop until through all ROM bytes 0-7
		
		// if the search was successful then
		if (!((id_bit_number < 65) || (crc8 != 0)))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;
			
			// check for last device
			if (LastDiscrepancy == 0)
				LastDeviceFlag = TRUE;
			
			search_result = TRUE;
		}
	}
	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || (ROM_NO[0] == 0))
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = FALSE;
		LastFamilyDiscrepancy = 0;
		search_result = FALSE;
	}
	return search_result;
}
