//
//  OWLevel.c
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
// Set the 1-Wire Net line level pull-up to normal. The DS2482 does only
// allows enabling strong pull-up on a bit or byte event. Consequently this
// function only allows the MODE_STANDARD argument. To enable strong pull-up
// use OWWriteBytePower or OWReadBitPower.  
//
// 'new_level' - new level defined as
//                MODE_STANDARD     0x00
//
// Returns:  current 1-Wire Net level
//
int OWLevel(int new_level)
{
	// function only will turn back to non-strong pull-up
	if (new_level != MODE_STANDARD)
		return MODE_STRONG;
	
	// clear the strong pull-up bit in the global config state
	cSPU = FALSE;
	
	// write the new config
	DS2482_write_config(c1WS | cSPU | cPPM | cAPU);
	
	return MODE_STANDARD;
}
