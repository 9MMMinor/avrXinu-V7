/**
 *  \file setTrxState.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 2/6/14.
//  Copyright (c) 2014.
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
//

#include <avr-Xinu.h>
#include <stdlib.h>
#include <util/delay.h>
#include <rfr2_radioDriver.h>
#include "radio.h"

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)

static Bool switch_pll_on(void);
void pauseMicroSeconds(void *, uint32_t);

uint8_t radio_TIMER = 9;
#define RADIO_TIMER &radio_TIMER

/********************************************************************************//*!
 * \fn radio_status_t radio_set_trx_state()
 * \brief Set the transceiver state.
 * \param uint8_t requestState
 * \return RADIO_SUCCESS if successful
 * \return else enum radioReturnValues
 ***********************************************************************************/

radio_status_t radio_set_trx_state(uint8_t requestState)
{
	uint8_t original_state = TRX_status.TRX_Status;
	radio_status_t status = RADIO_SUCCESS;
	
    switch (requestState)	{
		case STATE_RESET:
			/* forces the radio transceiver into the TRX_OFF state and resets all
			   transceiver register to their default values */
			TRX_pr.reset = 1;
			pauseMicroSeconds(RADIO_TIMER, TTR13+TTR14);	/* Figure 9-18 */
			return (status);
			
		case STATE_SLEEP:
			if (TRX_pr.sleep == 1)
				break;
			/* fall through */
			
        case STATE_TRX_OFF:
            switch (original_state)	{
				case STATUS_TRX_OFF:
					break;
				case STATUS_SLEEP:
					TRX_pr.sleep = 0;
					pauseMicroSeconds(RADIO_TIMER, TTR2);
					break;
				default:
					TRX_state.TRX_Cmd = CMD_FORCE_TRX_OFF;
					delay_us(1);
					break;
			}
			if (requestState == CMD_SLEEP)	{
				TRX_pr.sleep = 1;
				return (status);
			}
            break;
			
        case STATE_PLL_ON:
            switch (original_state)	{
				case STATUS_PLL_ON:
					break;
				case STATUS_TRX_OFF:
					if ( !switch_pll_on() )	{
						status = RADIO_TIMED_OUT;
						break;
					}
					/* else fall through */
				default:
					TRX_state.TRX_Cmd = CMD_PLL_ON;
					delay_us(1);
					break;
			}
            break;
						
        case STATE_RX_ON:
            switch (original_state)	{
				case STATUS_RX_ON:
					break;
				case STATUS_RX_AACK_ON:
				case STATUS_TX_ARET_ON:
					TRX_state.TRX_Cmd = CMD_TRX_OFF;
					delay_us(1);
					/* fall through */
				case STATUS_TRX_OFF:
					if ( !switch_pll_on() )	{
						status = RADIO_TIMED_OUT;
						break;
					}
					/* fall through */
				case STATUS_PLL_ON:
					TRX_state.TRX_Cmd = CMD_RX_ON;
					delay_us(1);
					break;
				default:
					ASSERT("state transition not handled" == 0);
					status = RADIO_STATE_TRANSITION_FAILED;
					break;
			}
            break;
			
        case STATE_RX_AACK_ON:
            switch (original_state)	{
				case STATUS_RX_AACK_ON:
					break;
				case STATUS_RX_ON:
				case STATUS_TX_ARET_ON:
					TRX_state.TRX_Cmd = CMD_PLL_ON;
					delay_us(1);
					/* fall through */
				case STATUS_PLL_ON:
					TRX_state.TRX_Cmd = CMD_RX_AACK_ON;
					delay_us(1);
					break;
				case STATUS_TRX_OFF:
					if ( !switch_pll_on() )	{
						status = RADIO_TIMED_OUT;
						break;
					}
					TRX_state.TRX_Cmd = CMD_RX_AACK_ON;
					delay_us(1);
					break;
				default:
					ASSERT("state transition not handled" == 0);
					status = RADIO_STATE_TRANSITION_FAILED;
					break;
			}
            break;
			
        case STATE_TX_ARET_ON:
            switch (original_state)	{
				case STATUS_TX_ARET_ON:
					break;
				case STATUS_RX_ON:
				case STATUS_RX_AACK_ON:
					TRX_state.TRX_Cmd = CMD_PLL_ON;
					delay_us(1);
					/* fall through */
				case STATUS_PLL_ON:
					TRX_state.TRX_Cmd = CMD_TX_ARET_ON;
					delay_us(1);
					break;
				case STATUS_TRX_OFF:
					if ( !switch_pll_on() )	{
						status = RADIO_TIMED_OUT;
						break;
					}
					TRX_state.TRX_Cmd = CMD_TX_ARET_ON;
					delay_us(1);
					break;
				default:
					ASSERT("state transition not handled" == 0);
					status = RADIO_STATE_TRANSITION_FAILED;
					break;
			}
            break;
			
        default:
            /* CMD_NOP */
            ASSERT("trx command not handled" == 0);
			status = RADIO_WRONG_STATE;
            break;
    }
	return (status);
}


/*
 * Switch the PLL on.  Called ONLY from TRX_OFF state.
 */
static Bool switch_pll_on(void)
{
    
	IRQ_status.PLL_Lock = 1;			/* clear PLL lock bit */
	TRX_state.TRX_Cmd = CMD_PLL_ON;
	pauseMicroSeconds(RADIO_TIMER, 2*TIME_PLL_LOCK);
	/* Check if PLL is locked */
	if (IRQ_status.PLL_Lock == 1)
		return (TRUE);  				/* PLL is locked now	*/
	ASSERT(IRQ_status.PLL_Lock == 1);	/* else TIMED-OUT		*/
	return (FALSE);
}
