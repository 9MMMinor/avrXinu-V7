//
//  OWSearch.c
//  1-Wire_SingleGPIO
//
//  Created by Michael Minor on 10/29/14.
//
//

//  Copyright (C) 2002 Dallas Semiconductor Corporation,
//  All Rights Reserved.
//
//  Permission is hereby granted, free of charge,
//  to any person obtaining a copy of this software and
//  associated documentation files (the "Software"), to
//  deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify,
//  merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom
//  the Software is furnished to do so, subject to the
//  following conditions:
//
//  The above copyright notice and this permission notice
//  shall be included in all copies or substantial portions
//  of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
//  ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
//  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//  Except as contained in this notice, the name of
//  Dallas Semiconductor shall not be used except as stated
//  in the Dallas Semiconductor Branding Policy.
//
//  APPLICATION NOTE 187 - 1-Wire Search Algorithm


#include <avr-Xinu.h>
#include <1-WireAPI.h>

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWSearch()
{
    int id_bit_number;
    int last_zero, rom_byte_number, search_result;
    int id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;
    
    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;
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
            // read a bit and its complement
            id_bit = OWReadBit();
            cmp_id_bit = OWReadBit();
            
            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else
            {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit;  // bit write value for search
                else
                {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);
                    
                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;
                        
                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }
                
                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;
                
                // serial number search direction write bit
                OWWriteBit(search_direction);
                
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
        while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7
        
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
    if (!search_result || !ROM_NO[0])
    {
        LastDiscrepancy = 0;
        LastDeviceFlag = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result = FALSE;
    }
    
    return search_result;
}
