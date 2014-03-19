/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *      Colin O'Flynn coflynn@newae.com
 *      Eric Gnoske egnoske@gmail.com
 *      Blake Leverett bleverett@gmail.com
 *      Mike Vidales mavida404@gmail.com
 *      Kevin Brown kbrown3@uccs.edu
 *      Nate Bohlmann nate@elfwerks.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
*/

/**
 *  \brief This module contains the radio driver code for the Atmel
 *  AT86RF230, '231, and '212 chips.
 *
 *  \author Blake Leverett <bleverett@gmail.com>
 *          Mike Vidales <mavida404@gmail.com>
 *          Eric Gnoske <egnoske@gmail.com>
 *
*/

/**  \addtogroup wireless
 * @{
 */

/**
 *  \defgroup radiorf230 RF230 interface
 * @{
 */
/**
 *  \file
 *  This file contains radio driver code.
 *
 */

/**
 *  \brief Modified driver code for the Atmel RFR2 Microprocessors.
 *  ATMega2564rfr2
 *
 *  \author Michael Minor <info.avrxinu@gmail.com>
 *
 */



/*============================ INCLUDE =======================================*/
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <stdlib.h>
#include <util/delay.h>
#include <rfr2_radioDriver.h>
#include "radio.h"

uint8_t pause_TIMER = 3;
#define PAUSE_TIMER &pause_TIMER

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)


/*============================ MACROS ========================================*/
#define RADIO_CCA_DONE_MASK     (1 << 7) /**<  Mask used to check the CCA_DONE bit. */
#define RADIO_CCA_IDLE_MASK     (1 << 6) /**<  Mask used to check the CCA_STATUS bit. */

#define RADIO_START_CCA (1) /**<  Value in the CCA_REQUEST subregister that initiate a cca. */

#define RADIO_TRANSMISSION_SUCCESS  (0)
#define RADIO_BUSY_CHANNEL          (3)
#define RADIO_MIN_IEEE_FRAME_LENGTH (5)
/*============================ TYPEDEFS ======================================*/

/** \brief  This enumeration defines the necessary timing information for the
 *          AT86RF230 radio transceiver. All times are in microseconds.
 *
 *          These constants are extracted from the datasheet.
 */
#ifndef _02_15_4_Mib_rfr2_radioDriver_h
typedef enum{
    TIME_TO_ENTER_P_ON               = 510, /**<  Transition time from VCC is applied to P_ON. */
    TIME_P_ON_TO_TRX_OFF             = 510, /**<  Transition time from P_ON to TRX_OFF. */
    TIME_SLEEP_TO_TRX_OFF            = 880, /**<  Transition time from SLEEP to TRX_OFF. */
    TIME_RESET                       = 6,   /**<  Time to hold the RST pin low during reset */
    TIME_ED_MEASUREMENT              = 140, /**<  Time it takes to do a ED measurement. */
    TIME_CCA                         = 140, /**<  Time it takes to do a CCA. */
    TIME_PLL_LOCK                    = 150, /**<  Maximum time it should take for the PLL to lock. */
    TIME_FTN_TUNING                  = 25,  /**<  Maximum time it should take to do the filter tuning. */
    TIME_NOCLK_TO_WAKE               = 6,   /**<  Transition time from *_NOCLK to being awake. */
    TIME_CMD_FORCE_TRX_OFF           = 1,    /**<  Time it takes to execute the FORCE_TRX_OFF command. */
    TIME_TRX_OFF_TO_PLL_ACTIVE       = 180, /**<  Transition time from TRX_OFF to: RX_ON, PLL_ON, TX_ARET_ON and RX_AACK_ON. */
    TIME_STATE_TRANSITION_PLL_ACTIVE = 1, /**<  Transition time from PLL active state to another. */
}radio_trx_timing_t;
#endif
/** \brief  This enumeration defines the necessary timing information for the
 *          rfr2 built-in radio transceiver. All times are in microseconds.
 *
 *          These constants are extracted from the datasheet.
 */

/*============================ VARIABLES =====================================*/
//static hal_rx_start_isr_event_handler_t user_rx_event;
//static hal_trx_end_isr_event_handler_t user_trx_end_event;
//static radio_rx_callback rx_frame_callback;
static uint8_t rssi_val;
//static uint8_t rx_mode;
//uint8_t rxMode = RX_AACK_ON;

/* See clock.c and httpd-cgi.c for RADIOSTATS code */
#if RADIOSTATS
uint8_t RADIO_radio_on, RADIO_rsigsi;
uint16_t RADIO_sendpackets,RADIO_receivepackets,RADIO_sendfail,RADIO_receivefail;
#endif

//static hal_rx_frame_t rx_frame;
//static parsed_frame_t parsed_frame;

/*============================ PROTOTYPES ====================================*/
void pauseMicroSeconds(void *, uint32_t);
Bool radio_is_sleeping(void);
//static void radio_rx_start_event(uint32_t const isr_timestamp, uint8_t const frame_length);
//static void radio_trx_end_event(uint32_t const isr_timestamp);
//static void switch_pll_on(void);

/** \brief  Initialize the Transceiver Access Toolbox and lower layers.
 *
 *          If the initialization is successful the radio transceiver will be in
 *          TRX_OFF state.
 *
 *  \note  This function must be called prior to any of the other functions in
 *         this file! Can be called from any transceiver state.
 *
 *  \param cal_rc_osc If true, the radio's accurate clock is used to calibrate the
 *                    CPU's internal RC oscillator.
 *
 *  \param rx_event Optional pointer to a user-defined function to be called on an
 *                  RX_START interrupt.  Use NULL for no handler.
 *
 *  \param trx_end_event Optional pointer to a user-defined function to be called on an
 *                  TRX_END interrupt.  Use NULL for no handler.
 *
 *  \param rx_callback Optional pointer to a user-defined function that receives
 *         a frame from the radio one byte at a time.  If the index parameter to
 *         this callback is 0xff, then the function should reset its state and prepare
 *         for a frame from the radio, with one call per byte.
 *
 *  \retval RADIO_SUCCESS     The radio transceiver was successfully initialized
 *                          and put into the TRX_OFF state.
 *  \retval RADIO_UNSUPPORTED_DEVICE  The connected device is not an Atmel
 *                                  AT86RF230 radio transceiver.
 *  \retval RADIO_TIMED_OUT   The radio transceiver was not able to initialize and
 *                          enter TRX_OFF state within the specified time.
 */

radio_status_t
//radio_init(bool cal_rc_osc,
//           hal_rx_start_isr_event_handler_t rx_event,
//           hal_trx_end_isr_event_handler_t trx_end_event,
//           radio_rx_callback rx_callback)
radio_init(void)
{
    radio_status_t init_status = RADIO_SUCCESS;

//	delay_us(TIME_TO_ENTER_P_ON);
	pauseMicroSeconds(PAUSE_TIMER, TIME_TO_ENTER_P_ON);

    /*  calibrate oscillator */
//    if (cal_rc_osc){
//        calibrate_rc_osc_32k();
//   }

    /* Initialize Hardware Abstraction Layer. */
//    hal_init();

    radio_reset_trx(); /* Do HW reset of radio tranceiver. */

    /* Force transition to TRX_OFF. */

	TRX_state.TRX_Cmd = CMD_FORCE_TRX_OFF;
//	delay_us(TIME_P_ON_TO_TRX_OFF); /* Wait for the transition to be complete. */
	pauseMicroSeconds(PAUSE_TIMER, TIME_P_ON_TO_TRX_OFF);

    if (TRX_status.TRX_Status != STATUS_TRX_OFF)	{
		printf("Error: State=0x%0x\n",TRX_status.TRX_Status);
        init_status = RADIO_TIMED_OUT;
    } else {
        /* Read Version Number */
		uint8_t version_number = VERSION_num.num;

        if ( !(version_number == RADIO_REVD || version_number == RADIO_REVC) )	{
			printf("Error: Version number = 0x%0x\n", version_number);
            init_status = RADIO_UNSUPPORTED_DEVICE;
		}
        else {
            if (MAN_id0.id != SUPPORTED_MANUFACTURER_ID)	{
				printf("Error: manufacturer ID= 0x%x\n", MAN_id0.id);
                init_status = RADIO_UNSUPPORTED_DEVICE;
			}
            else	{
				IRQ_mask.RX_START_Enable = 0;
				IRQ_mask.RX_END_Enable = 1;			/* enable RX_END interrupt */
				IRQ_mask.TX_END_Enable = 0;
			}
        }
#if RADIOSTATS
        RADIO_radio_on = 1;
#endif
    }
	IRQ_status.TX_End = 1;					/* clears TX_End bit   */
	IRQ_mask.TX_END_Enable = 1;				/* enable TX interrupt */
	
	ASSERT(TRX_status.TRX_Status == STATUS_TRX_OFF);

//		set callbacks for events.  Save user's rx_event, which we will
//		call from radio_rx_start_event().  Same with trx_end
//		user_rx_event = rx_event;
//		user_trx_end_event = trx_end_event;
//		hal_set_rx_start_event_handler(radio_rx_start_event);
//		hal_set_trx_end_event_handler(radio_trx_end_event);
//		rx_frame_callback = rx_callback;

    return init_status;
}

/*---------------------------------------------------------------------------*/
//uint8_t *
//radio_frame_data(void)
//{
//        return rx_frame.data;
//}

//uint8_t
//radio_frame_length(void)
//{
//        return rx_frame.length;
//}

/*---------------------------------------------------------------------------*/
/*static void
radio_rx_start_event(uint32_t const isr_timestamp, uint8_t const frame_length)
{
    //  save away RSSI
    rssi_val =  hal_subregister_read( SR_RSSI );

    //  call user's rx_start event handler
    if (user_rx_event)
        user_rx_event(isr_timestamp, frame_length);
}
 */

/*---------------------------------------------------------------------------*/
uint8_t
radio_get_saved_rssi_value(void)
{
    return rssi_val;
}

/*---------------------------------------------------------------------------*/
/*static void
radio_trx_end_event(uint32_t const isr_timestamp)
{
    volatile uint8_t status;

    //  call user's trx_end event handler
    if (user_trx_end_event){
        user_trx_end_event(isr_timestamp);
        return;
    }
    if (rx_mode){
        // radio has received frame, store it away
#if RADIOSTATS
        RADIO_rsigsi=rssi_val;
        RADIO_receivepackets++;
#endif
        parsed_frame.time = isr_timestamp;
        parsed_frame.rssi = rssi_val;
        
        hal_frame_read(&rx_frame, NULL);
        rx_frame_parse(&rx_frame, &parsed_frame);
        }

    if (!rx_mode){
        //  Put radio back into receive mode.
        radio_set_trx_state(TRX_OFF);
        radio_set_trx_state(rxMode);

        //  transmit mode, put end-of-transmit event in queue
        event_object_t event;
        event.event = 0;
        event.data = 0;
        status = hal_subregister_read(SR_TRAC_STATUS);
        switch(status){
        case TRAC_SUCCESS:
        case TRAC_SUCCESS_DATA_PENDING:
            event.event = MAC_EVENT_ACK;
            break;
        case TRAC_NO_ACK:
        case TRAC_CHANNEL_ACCESS_FAILURE:
            event.event = MAC_EVENT_NACK;
#if RADIOSTATS
        RADIO_sendfail++;
#endif
            break;
        case TRAC_SUCCESS_WAIT_FOR_ACK:
            //  should only happen in RX mode
        case TRAC_INVALID:
            //  should never happen here
        default:
#if RADIOSTATS
            RADIO_sendfail++;
#endif
            break;
        }
        if (event.event)
            mac_put_event(&event);
        process_post(&mac_process, event.event, event.data);
    }
}
*/
 
/*----------------------------------------------------------------------------*/
/** \brief  This function will return the channel used by the radio transceiver.
 *
 *  \return Current channel, 11 to 26.
 */
uint8_t
radio_get_operating_channel(void)
{
    return PHY_CC_cca.Channel;
}
/*----------------------------------------------------------------------------*/
/** \brief This function will change the operating channel.
 *
 *  \param  channel New channel to operate on. Must be between 11 and 26.
 *
 *  \retval RADIO_SUCCESS New channel set.
 *  \retval RADIO_WRONG_STATE Transceiver is in a state where the channel cannot
 *                          be changed (SLEEP).
 *  \retval RADIO_INVALID_ARGUMENT Channel argument is out of bounds.
 *  \retval RADIO_TIMED_OUT The PLL did not lock within the specified time.
 */
radio_status_t
radio_set_operating_channel(uint8_t channel)
{
    /*Do function parameter and state check.*/
    if ((channel < RADIO_MIN_CHANNEL) ||
        (channel > RADIO_MAX_CHANNEL)){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true)	{
        return RADIO_WRONG_STATE;
    }

    if (radio_get_operating_channel() == channel)	{
        return RADIO_SUCCESS;
    }

    /*Set new operating channel.*/
    PHY_CC_cca.Channel = channel;

    /* Read current state and wait for the PLL_LOCK interrupt if the */
    /* radio transceiver is in either RX_ON or PLL_ON. */
    uint8_t trx_state = TRX_status.TRX_Status;

    if ((trx_state == STATUS_RX_ON) ||
        (trx_state == STATUS_PLL_ON)){
        delay_us(TIME_PLL_LOCK);
    }

    radio_status_t channel_set_status = RADIO_TIMED_OUT;

    /* Check that the channel was set properly. */
    if (radio_get_operating_channel() == channel)	{
        channel_set_status = RADIO_SUCCESS;
    }

    return channel_set_status;
}

/*----------------------------------------------------------------------------*/
/** \brief This function will read and return the output power level.
 *
 *  \returns 0 to 15 Current output power in "TX power settings" as defined in
 *          the radio transceiver's datasheet
 */
uint8_t
radio_get_tx_power_level(void)
{
    return PHY_TX_pwr.Pwr;
}

/*----------------------------------------------------------------------------*/
/** \brief This function will change the output power level.
 *
 *  \param  power_level New output power level in the "TX power settings"
 *                      as defined in the radio transceiver's datasheet.
 *
 *  \retval RADIO_SUCCESS New output power set successfully.
 *  \retval RADIO_INVALID_ARGUMENT The supplied function argument is out of bounds.
 *  \retval RADIO_WRONG_STATE It is not possible to change the TX power when the
 *                          device is sleeping.
 */
radio_status_t
radio_set_tx_power_level(uint8_t power_level)
{

    /*Check function parameter and state.*/
    if (power_level > TX_PWR_17_2DBM){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Set new power level*/
    PHY_TX_pwr.Pwr = power_level;

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current CCA mode used.
 *
 *  \return CCA mode currently used, 0 to 3.
 */
uint8_t
radio_get_cca_mode(void)
{
    return PHY_CC_cca.CCA_Mode;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current ED threshold used by the CCA algorithm.
 *
 *  \return Current ED threshold, 0 to 15.
 */
uint8_t
radio_get_ed_threshold(void)
{
    return CCA_thres.ED_threshold;
}

/*----------------------------------------------------------------------------*/
/** \brief This function will configure the Clear Channel Assessment algorithm.
 *
 *  \param  mode Three modes are available: Energy above threshold, carrier
 *               sense only and carrier sense with energy above threshold.
 *  \param  ed_threshold Above this energy threshold the channel is assumed to be
 *                      busy. The threshold is given in positive dBm values.
 *                      Ex. -91 dBm gives a csThreshold of 91. Value range for
 *                      the variable is [61 to 91]. Only valid for the CCA_ED
 *                      and CCA_CARRIER_SENSE_ED modes.
 *
 *  \retval RADIO_SUCCESS Mode and its parameters successfully changed.
 *  \retval RADIO_WRONG_STATE This function cannot be called in the SLEEP state.
 *  \retval RADIO_INVALID_ARGUMENT If one of the three function arguments are out
 *                               of bounds.
 */
radio_status_t
radio_set_cca_mode(uint8_t mode, uint8_t ed_threshold)
{
    /*Check function parameters and state.*/
    if ((mode != CCA_ED) &&
        (mode != CCA_CARRIER_SENSE) &&
        (mode != CCA_CARRIER_SENSE_WITH_ED)){
        return RADIO_INVALID_ARGUMENT;
    }

    /* Ensure that the ED threshold is within bounds. */
    if (ed_threshold > RADIO_MAX_ED_THRESHOLD){
        return RADIO_INVALID_ARGUMENT;
    }

    /* Ensure that the radio transceiver is not sleeping. */
    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Change cca mode and ed threshold.*/

	PHY_CC_cca.CCA_Mode = mode;
	CCA_thres.ED_threshold = ed_threshold;

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the Received Signal Strength Indication.
 *
 *  \note This function should only be called from the: RX_ON and BUSY_RX. This
 *        can be ensured by reading the current state of the radio transceiver
 *        before executing this function!
 *  \param rssi Pointer to memory location where RSSI value should be written.
 *  \retval RADIO_SUCCESS The RSSI measurement was successful.
 *  \retval RADIO_WRONG_STATE The radio transceiver is not in RX_ON or BUSY_RX.
 */
radio_status_t
radio_get_rssi_value(uint8_t *rssi)
{

    uint8_t current_state = TRX_status.TRX_Status;
    radio_status_t retval = RADIO_WRONG_STATE;

    /*The RSSI measurement should only be done in RX_ON or BUSY_RX.*/
    if ((current_state == STATUS_RX_ON) ||
        (current_state == STATUS_BUSY_RX))	{
		*rssi = PHY_rssi.Rssi;
        retval = RADIO_SUCCESS;
    }

    return retval;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current threshold volatge used by the
 *         battery monitor (BATMON_VTH).
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \return Current threshold voltage, 0 to 15.
 */
uint8_t
radio_batmon_get_voltage_threshold(void)
{
	return PHY_batmon.BATMON_Vth;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns if high or low voltage range is used.
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \retval 0 Low voltage range selected.
 *  \retval 1 High voltage range selected.
 */
uint8_t
radio_batmon_get_voltage_range(void)
{
	return PHY_batmon.BATMON_Hr;
}

/*----------------------------------------------------------------------------*/
/** \brief This function is used to configure the battery monitor module
 *
 *  \param range True means high voltage range and false low voltage range.
 *  \param voltage_threshold The datasheet defines 16 voltage levels for both
 *                          low and high range.
 *  \retval RADIO_SUCCESS Battery monitor configured
 *  \retval RADIO_WRONG_STATE The device is sleeping.
 *  \retval RADIO_INVALID_ARGUMENT The voltage_threshold parameter is out of
 *                               bounds (Not within [0 - 15]).
 */
radio_status_t
radio_batmon_configure(bool range, uint8_t voltage_threshold)
{

    /*Check function parameters and state.*/
    if (voltage_threshold > BATTERY_MONITOR_HIGHEST_VOLTAGE){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Write new voltage range and voltage level.*/
    if (range == true){
		PHY_batmon.BATMON_Hr = BATTERY_MONITOR_HIGH_VOLTAGE;
    } else {
		PHY_batmon.BATMON_Hr = BATTERY_MONITOR_LOW_VOLTAGE;
    }

	PHY_batmon.BATMON_Vth = voltage_threshold;

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the status of the Battery Monitor module.
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \retval RADIO_BAT_LOW Battery voltage is below the programmed threshold.
 *  \retval RADIO_BAT_OK Battery voltage is above the programmed threshold.
 */
radio_status_t
radio_batmon_get_status(void)
{

    radio_status_t batmon_status = RADIO_BAT_LOW;

	if (PHY_batmon.BATMON_Ok != BATTERY_MONITOR_VOLTAGE_UNDER_THRESHOLD) {
        batmon_status = RADIO_BAT_OK;
    }

    return batmon_status;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current clock setting for the CLKM pin.
 *
 *  \retval CLKM_DISABLED CLKM pin is disabled.
 *  \retval CLKM_1MHZ CLKM pin is prescaled to 1 MHz.
 *  \retval CLKM_2MHZ CLKM pin is prescaled to 2 MHz.
 *  \retval CLKM_4MHZ CLKM pin is prescaled to 4 MHz.
 *  \retval CLKM_8MHZ CLKM pin is prescaled to 8 MHz.
 *  \retval CLKM_16MHZ CLKM pin is not prescaled. Output is 16 MHz.
 */
uint8_t
radio_get_clock_speed(void)
{
    return TRX_ctrl0.ctrl0;		/*reserved for mega2564rfr2*/
}

/*----------------------------------------------------------------------------*/
/** \brief This function changes the prescaler on the CLKM pin.
 *
 *  \param direct   This boolean variable is used to determine if the frequency
 *                  of the CLKM pin shall be changed directly or not. If direct
 *                  equals true, the frequency will be changed directly. This is
 *                  fine if the CLKM signal is used to drive a timer etc. on the
 *                  connected microcontroller. However, the CLKM signal can also
 *                  be used to clock the microcontroller itself. In this situation
 *                  it is possible to change the CLKM frequency indirectly
 *                  (direct == false). When the direct argument equlas false, the
 *                  CLKM frequency will be changed first after the radio transceiver
 *                  has been taken to SLEEP and awaken again.
 *  \param clock_speed This parameter can be one of the following constants:
 *                     CLKM_DISABLED, CLKM_1MHZ, CLKM_2MHZ, CLKM_4MHZ, CLKM_8MHZ
 *                     or CLKM_16MHZ.
 *
 *  \retval RADIO_SUCCESS Clock speed updated. New state is TRX_OFF.
 *  \retval RADIO_INVALID_ARGUMENT Requested clock speed is out of bounds.
 */
radio_status_t
radio_set_clock_speed(bool direct, uint8_t clock_speed)
{
	
    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function calibrates the Single Side Band Filter.
 *
 *  \retval     RADIO_SUCCESS    Filter is calibrated.
 *  \retval     RADIO_TIMED_OUT  The calibration could not be completed within time.
 *  \retval     RADIO_WRONG_STATE This function can only be called from TRX_OFF or
 *              PLL_ON.
 */
radio_status_t
radio_calibrate_filter(void)
{
    /*Check current state. Only possible to do filter calibration from TRX_OFF or PLL_ON.*/
    uint8_t trx_state = TRX_status.TRX_Status;

    if ((trx_state != STATUS_TRX_OFF) &&
        (trx_state != STATUS_PLL_ON)){
        return RADIO_WRONG_STATE;
    }

    /* Start the tuning algorithm by writing one to the FTN_START subregister. */
	FTN_ctrl.ftn_start = 1;
    delay_us(TIME_FTN_TUNING); /* Wait for the calibration to finish. */

    radio_status_t filter_calibration_status = RADIO_TIMED_OUT;

    /* Verify the calibration result. */
    if (FTN_ctrl.ftn_start == FTN_CALIBRATION_DONE){
        filter_calibration_status = RADIO_SUCCESS;
    }

    return filter_calibration_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function calibrates the PLL.
 *
 *  \retval     RADIO_SUCCESS    PLL Center Frequency and Delay Cell is calibrated.
 *  \retval     RADIO_TIMED_OUT  The calibration could not be completed within time.
 *  \retval     RADIO_WRONG_STATE This function can only be called from PLL_ON.
 */
radio_status_t
radio_calibrate_pll(void)
{

    /*Check current state. Only possible to calibrate PLL from PLL_ON state*/
    if (TRX_status.TRX_Status != STATUS_PLL_ON)	{
        return RADIO_WRONG_STATE;
    }

    /* Initiate the DCU and CF calibration loops. */
	PLL_dcu.start = 1;
	PLL_cf.start = 1;

    /* Wait maximum 150 us for the PLL to lock. */
//    hal_clear_pll_lock_flag();
    delay_us(TIME_PLL_LOCK);

    radio_status_t pll_calibration_status = RADIO_TIMED_OUT;

//    if (hal_get_pll_lock_flag() > 0){
        if (PLL_dcu.start == PLL_DCU_CALIBRATION_DONE)	{
            if (PLL_cf.start == PLL_CF_CALIBRATION_DONE)	{
                pll_calibration_status = RADIO_SUCCESS;
            }
        }
    return pll_calibration_status;
}


/*----------------------------------------------------------------------------*/
/** \brief  This function checks if the radio transceiver is sleeping.
 *
 *  \retval     true    The radio transceiver is in SLEEP or one of the *_NOCLK
 *                      states.
 *  \retval     false   The radio transceiver is not sleeping.
 */
Bool radio_is_sleeping(void)
{
    Bool sleeping = false;

    /* The radio transceiver will be at SLEEP or one of the *_NOCLK states only if */
    /* the SLP_TR pin is high. */
    if (TRX_pr.sleep == 1){
        sleeping = true;
    }

    return sleeping;
}


/** \brief  This function will change the current state of the radio
 *          transceiver's internal state machine.
 *
 *  \param     new_state        Here is a list of possible states:
 *             - RX_ON        Requested transition to RX_ON state.
 *             - TRX_OFF      Requested transition to TRX_OFF state.
 *             - PLL_ON       Requested transition to PLL_ON state.
 *             - RX_AACK_ON   Requested transition to RX_AACK_ON state.
 *             - TX_ARET_ON   Requested transition to TX_ARET_ON state.
 *
 *  \retval    RADIO_SUCCESS          Requested state transition completed
 *                                  successfully.
 *  \retval    RADIO_INVALID_ARGUMENT Supplied function parameter out of bounds.
 *  \retval    RADIO_WRONG_STATE      Illegal state to do transition from.
 *  \retval    RADIO_BUSY_STATE       The radio transceiver is busy.
 *  \retval    RADIO_TIMED_OUT        The state transition could not be completed
 *                                  within resonable time.
 */

/*
radio_status_t
radio_set_trx_state(uint8_t new_state)
{
    uint8_t original_state;

    //Check function paramter and current state of the radio transceiver.
    if (!((new_state == STATUS_TRX_OFF)    ||
          (new_state == STATUS_RX_ON)      ||
          (new_state == STATUS_PLL_ON)     ||
          (new_state == STATUS_RX_AACK_ON) ||
          (new_state == STATUS_TX_ARET_ON)))	{
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    // Wait for radio to finish previous operation
    for(;;)
    {
        original_state = TRX_status.TRX_Status;
        if (original_state != STATUS_BUSY_TX_ARET &&
            original_state != STATUS_BUSY_RX_AACK &&
            original_state != STATUS_BUSY_RX && 
            original_state != STATUS_BUSY_TX)
            break;
    }

    if (new_state == original_state){
        return RADIO_SUCCESS;
    }


    // At this point it is clear that the requested new_state is:
    // TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON.

    // The radio transceiver can be in one of the following states:
    // TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON.
    if(new_state == STATUS_TRX_OFF){
        radio_reset_state_machine(); // Go to TRX_OFF from any state.
    } else {
        // It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to
        // TX_AACK_ON or RX_AACK_ON respectively. Need to go via RX_ON or PLL_ON.
        if ((new_state == STATUS_TX_ARET_ON) &&
            (original_state == STATUS_RX_AACK_ON)){
            // First do intermediate state transition to PLL_ON, then to TX_ARET_ON.
            // The final state transition to TX_ARET_ON is handled after the if-else if.
			while (TRX_status.TRX_Status==STATE_TRANSITION_IN_PROGRESS)
				; // wait
			TRX_state.TRX_Cmd = CMD_PLL_ON;
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        } else if ((new_state == STATUS_RX_AACK_ON) &&
                 (original_state == STATUS_TX_ARET_ON))	{
            // First do intermediate state transition to RX_ON, then to RX_AACK_ON.
            // The final state transition to RX_AACK_ON is handled after the if-else if.
			while (TRX_status.TRX_Status==STATUS_STATE_TRANSITION_IN_PROGRESS)
				; // wait
			TRX_state.TRX_Cmd = CMD_RX_ON;
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }

        // Any other state transition can be done directly.
		while (TRX_status.TRX_Status==STATE_TRANSITION_IN_PROGRESS)
			; // wait
		if (new_state == CMD_PLL_ON && original_state == STATUS_TRX_OFF)	{
			switch_pll_on();
		}
		else	{
			TRX_state.TRX_Cmd = new_state;
		}

        // When the PLL is active most states can be reached in 1us.
        if (original_state != STATUS_TRX_OFF)	{
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
    } // end: if(new_state == TRX_OFF) ...

    //Verify state transition.
    radio_status_t set_state_status = RADIO_TIMED_OUT;

    if (TRX_status.TRX_Status == new_state){
        set_state_status = RADIO_SUCCESS;
        // set rx_mode flag based on mode we're changing to
        if (new_state == STATUS_RX_ON ||
            new_state == STATUS_RX_AACK_ON)	{
            rx_mode = true;
        } else {
            rx_mode = false;
		}
    }

    return set_state_status;
}
 */

/*
 * Switch the PLL on.  Called ONLY from TRX_OFF state.
 */

/*
static void switch_pll_on(void)
{
	
    // Check if trx is in TRX_OFF; only from PLL_ON the following procedure is applicable
	if ( TRX_status.TRX_Status != STATUS_TRX_OFF )	{
		ASSERT("Switch PLL_ON failed, because trx is not in TRX_OFF" == 0);
		return;
    }
	
	IRQ_status.PLL_Lock = 1;			// clear PLL lock bit
	TRX_state.TRX_Cmd = CMD_PLL_ON;
	pauseMicroSeconds(PAUSE_TIMER, 2*TIME_PLL_LOCK);	// Check if PLL is locked
	if (IRQ_status.PLL_Lock == 1)	{
		return;  						// PLL is locked now
	}
	ASSERT(IRQ_status.PLL_Lock == 1);	// else TIMED-OUT
}
 */

/*----------------------------------------------------------------------------*/
/** \brief  This function will put the radio transceiver to sleep.
 *
 *  \retval    RADIO_SUCCESS          Sleep mode entered successfully.
 *  \retval    RADIO_TIMED_OUT        The transition to TRX_OFF took too long.
 */
radio_status_t
radio_enter_sleep_mode(void)
{
    if (radio_is_sleeping() == true)	{
        return RADIO_SUCCESS;
    }

    radio_reset_state_machine(); /* Force the device into TRX_OFF. */

    radio_status_t enter_sleep_status = RADIO_TIMED_OUT;

    if (TRX_status.TRX_Status == STATUS_TRX_OFF){
        /* Enter Sleep. */
        TRX_pr.sleep = 1;
        enter_sleep_status = RADIO_SUCCESS;
#if RADIOSTATS
        RADIO_radio_on = 0;
#endif
    }

    return enter_sleep_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will take the radio transceiver from sleep mode and
 *          put it into the TRX_OFF state.
 *
 *  \retval    RADIO_SUCCESS          Left sleep mode and entered TRX_OFF state.
 *  \retval    RADIO_TIMED_OUT        Transition to TRX_OFF state timed out.
 */
radio_status_t
radio_leave_sleep_mode(void)
{
    /* Check if the radio transceiver is actually sleeping. */
    if (radio_is_sleeping() == false){
        return RADIO_SUCCESS;
    }

    TRX_pr.sleep = 0;
    delay_us(TIME_SLEEP_TO_TRX_OFF);

    radio_status_t leave_sleep_status = RADIO_TIMED_OUT;

    /* Ensure that the radio transceiver is in the TRX_OFF state. */
    if (TRX_status.TRX_Status == STATUS_TRX_OFF){
        leave_sleep_status = RADIO_SUCCESS;
#if RADIOSTATS
        RADIO_radio_on = 1;
#endif
    }

    return leave_sleep_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will reset the state machine (to TRX_OFF) from any of
 *          its states, except for the SLEEP state.
 */
void
radio_reset_state_machine(void)
{
    TRX_pr.sleep = 0;
    delay_us(TIME_NOCLK_TO_WAKE);
	TRX_state.TRX_Cmd = CMD_FORCE_TRX_OFF;
    delay_us(TIME_CMD_FORCE_TRX_OFF);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will reset all the registers and the state machine of
 *          the radio transceiver.
 */
void
radio_reset_trx(void)
{
    TRX_pr.reset = 0;
    TRX_pr.sleep = 0;
	delay_us(TIME_RESET);
    TRX_pr.reset = 1;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will enable or disable automatic CRC during frame
 *          transmission.
 *
 *  \param  auto_crc_on If this parameter equals true auto CRC will be used for
 *                      all frames to be transmitted. The framelength must be
 *                      increased by two bytes (16 bit CRC). If the parameter equals
 *                      false, the automatic CRC will be disabled.
 */
void
radio_use_auto_tx_crc(bool auto_crc_on)
{
    if (auto_crc_on == true){
		TRX_ctrl1.TX_AUTO_CRC_On = 1;
    } else {
		TRX_ctrl1.TX_AUTO_CRC_On = 0;
    }
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's
 *          transmit buffer and send it.
 *
 *  \param  data_length Length of the frame to be transmitted. 1 to 128 bytes are the valid lengths.
 *  \param  *data   Pointer to the data to transmit
 *
 *  \retval RADIO_SUCCESS Frame downloaded and sent successfully.
 *  \retval RADIO_INVALID_ARGUMENT If the dataLength is 0 byte or more than 127
 *                               bytes the frame will not be sent.
 *  \retval RADIO_WRONG_STATE It is only possible to use this function in the
 *                          PLL_ON and TX_ARET_ON state. If any other state is
 *                          detected this error message will be returned.
 */
radio_status_t
radio_send_data(uint8_t data_length, uint8_t *data)
{
#if ( defined (XINU_TARGET_MEGA2564RF) || defined (XINU_TARGET_256RFR2XPLAINPRO) )

	return (write(RADIO, data, data_length) > 0);
#else
    /*Check function parameters and current state.*/
    if (data_length > RADIO_MAX_TX_FRAME_LENGTH){
#if RADIOSTATS
        RADIO_sendfail++;
#endif
        return RADIO_INVALID_ARGUMENT;
    }

     /* If we are busy, return */
        if ((TRX_status.TRX_Status == BUSY_TX) || (TRX_status.TRX_Status == BUSY_TX_ARET) )
        {
#if RADIOSTATS
        RADIO_sendfail++;
#endif
        return RADIO_WRONG_STATE;
        }

    radio_set_trx_state(TRX_OFF);
    radio_set_trx_state(TX_ARET_ON);

    /*Do frame transmission.*/
    /* Toggle the SLP_TR pin to initiate the frame transmission. */
    TRX_pr.sleep = 1;
    TRX_pr.sleep = 0;

    hal_frame_write(data, data_length); /* Then write data to the frame buffer. */

#if RADIOSTATS
    RADIO_sendpackets++;
#endif
    return RADIO_SUCCESS;
#endif
	
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will read the I_AM_COORD sub register.
 *
 *  \retval 0 Not coordinator.
 *  \retval 1 Coordinator role enabled.
 */
uint8_t
radio_get_device_role(void)
{
    return CSMA_seed1.AACK_I_AM_Coord;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the I_AM_COORD sub register.
 *
 *  \param[in] i_am_coordinator If this parameter is true, the associated
 *                              coordinator role will be enabled in the radio
 *                              transceiver's address filter.
 *                              False disables the same feature.
 */
void
radio_set_device_role(bool i_am_coordinator)
{
    CSMA_seed1.AACK_I_AM_Coord = i_am_coordinator;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will return the PANID used by the address filter.
 *
 *  \retval Any value from 0 to 0xFFFF.
 */
uint16_t
radio_get_pan_id(void)
{

    uint8_t pan_id_15_8 = PAN_id1.id; /*  Read pan_id_15_8. */
    uint8_t pan_id_7_0 = PAN_id0.id; /*  Read pan_id_7_0. */

    uint16_t pan_id = ((uint16_t)(pan_id_15_8 << 8)) | pan_id_7_0;

    return pan_id;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the PANID used by the address filter.
 *
 *  \param  new_pan_id Desired PANID. Can be any value from 0x0000 to 0xFFFF
 */
void
radio_set_pan_id(uint16_t new_pan_id)
{

    uint8_t pan_byte = new_pan_id & 0xFF; /*  Extract new_pan_id_7_0. */
 //   hal_register_write(RG_PAN_ID_0, pan_byte);
	PAN_id0.id = pan_byte;

    pan_byte = (new_pan_id >> 8*1) & 0xFF;  /*  Extract new_pan_id_15_8. */
 //   hal_register_write(RG_PAN_ID_1, pan_byte);
	PAN_id1.id = pan_byte;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will return the current short address used by the
 *          address filter.
 *
 *  \retval Any value from 0x0000 to 0xFFFF
 */
uint16_t
radio_get_short_address(void)
{

    uint8_t short_address_15_8 = SHORT_addr1.address; /*  Read short_address_15_8. */
    uint8_t short_address_7_0  = SHORT_addr0.address; /*  Read short_address_7_0. */

    uint16_t short_address = ((uint16_t)(short_address_15_8 << 8)) | short_address_7_0;

    return short_address;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the short address used by the address filter.
 *
 *  \param  new_short_address Short address to be used by the address filter.
 */
void
radio_set_short_address(uint16_t new_short_address)
{

    uint8_t short_address_byte = new_short_address & 0xFF; /*  Extract short_address_7_0. */
	SHORT_addr0.address = short_address_byte;

    short_address_byte = (new_short_address >> 8*1) & 0xFF; /*  Extract short_address_15_8. */
	SHORT_addr1.address = short_address_byte;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will read the extended address used by the address
 *          filter.
 *
 *  \note In this function a pointer is used to convey the 64-bit result, since
 *        it is very inefficient to use the stack for this.
 *
 *  \return Extended Address, any 64-bit value.
 */
void
radio_get_extended_address(uint8_t *extended_address)
{
    *extended_address++ = IEEE_addr0.address;
    *extended_address++ = IEEE_addr1.address;
    *extended_address++ = IEEE_addr2.address;
    *extended_address++ = IEEE_addr3.address;
    *extended_address++ = IEEE_addr4.address;
    *extended_address++ = IEEE_addr5.address;
    *extended_address++ = IEEE_addr6.address;
    *extended_address   = IEEE_addr7.address;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set a new extended address to be used by the
 *          address filter.
 *
 *  \param  extended_address Extended address to be used by the address filter.
 */
void
radio_set_extended_address(uint8_t *extended_address)
{
    IEEE_addr0.address = *extended_address++;
    IEEE_addr1.address = *extended_address++;
    IEEE_addr2.address = *extended_address++;
    IEEE_addr3.address = *extended_address++;
    IEEE_addr4.address = *extended_address++;
    IEEE_addr5.address = *extended_address++;
    IEEE_addr6.address = *extended_address++;
    IEEE_addr7.address = *extended_address;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will configure the CSMA algorithm used by the radio
 *          transceiver when transmitting data from TX_ARET_ON state.
 *
 *  \param  seed0 Lower 8 bits of the seed used for the random number generator
 *                in the CSMA algorithm. Value range: 0 to 255.
 *  \param  be_csma_seed1 Is a combined argument of the MIN_BE, MAX_CSMA_RETRIES
 *                        and SEED1 variables:
 *                        -# MIN_BE: Bit[7:6] Minimum back-off exponent in the
 *                           CSMA/CA algorithm.
 *                        -# MAX_CSMA_RETRIES: Bit[5:3] Number of retries in
 *                          TX_ARET_ON mode to repeat the CSMA/CA procedures
 *                          before the ARET procedure gives up.
 *                        -# SEED1: Bits[2:0] Higher 3 bits of CSMA_SEED, bits[10:8]
 *                           Seed for the random number generator in the
 *                           CSMA/CA algorithm.
 *  \retval RADIO_SUCCESS The CSMA algorithm was configured successfully.
 *  \retval RADIO_WRONG_STATE This function should not be called in the
 *                          SLEEP state.
 */
radio_status_t
radio_configure_csma(uint8_t seed0, uint8_t be_csma_seed1)
{

    /*Check state.*/
    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Extract parameters, and configure the CSMA-CA algorithm.*/
    uint8_t back_off_exponent = (be_csma_seed1 & 0xC0) >> 6;
    uint8_t csma_retries      = (be_csma_seed1 & 0x38) >> 3;
    uint8_t seed1             = (be_csma_seed1 & 0x07);
	
	XAH_ctrl0.MAX_FRAME_Retries = 0;
	XAH_ctrl0.MAX_CSMA_Retries = csma_retries;
    CSMA_be.MIN_Be = back_off_exponent;
    CSMA_seed0.value = seed0;
    CSMA_seed1.value = seed1;

    return RADIO_SUCCESS;
}
