//
//  radioStatusString.c
//  radio-15-4
//
//  Created by Michael M Minor on 11/18/13.
//
//

//TO DO
// return pointer to malloc'ed RAM,
// caller free's it.

#include <stdio.h>
#include <avr/pgmspace.h>
#include <rfr2_radioDriver.h>

const char radioSuccess[] PROGMEM = "success.";
const char unsupportedDevice[] PROGMEM = "The connected device is not supported.";
const char invalidArgument[] PROGMEM = "One or more of the supplied function arguments are invalid.";
const char timedOut[] PROGMEM = "The requested service timed out.";
const char wrongState[] PROGMEM = "The end-user tried to do an invalid state transition.";
const char busyState[] PROGMEM = "The radio transceiver is busy receiving or transmitting.";
const char stateTransitionFail[] PROGMEM = "The requested state transition could not be completed.";
const char CCAIdle[] PROGMEM = "Channel is clear, available to transmit a new frame.";
const char CCABusy[] PROGMEM = "Channel busy.";
const char TRXBusy[] PROGMEM = "Transceiver is busy receiving or transmitting data.";
const char batLow[] PROGMEM = "Measured battery voltage is lower than voltage threshold.";
const char batOK[] PROGMEM = "Measured battery voltage is above the voltage threshold.";
const char CRCFailed[] PROGMEM = "The CRC failed for the actual frame.";
const char channelAccessFail[] PROGMEM = "The channel access failed during the auto mode.";
const char noAck[] PROGMEM = "No acknowledge frame was received.";
const char unKnown[] PROGMEM = "unknown";

const char *
radioStatusString(radio_status_t status)
{
	
	switch (status) {
        case RADIO_SUCCESS:
			return radioSuccess;
		case RADIO_UNSUPPORTED_DEVICE:
			return unsupportedDevice;
		case RADIO_INVALID_ARGUMENT:
			return invalidArgument;
		case RADIO_TIMED_OUT:
			return timedOut;
		case RADIO_WRONG_STATE:
			return wrongState;
		case RADIO_BUSY_STATE:
			return busyState;
		case RADIO_STATE_TRANSITION_FAILED:
			return stateTransitionFail;
		case RADIO_CCA_IDLE:
			return CCAIdle;
		case RADIO_CCA_BUSY:
			return CCABusy;
		case RADIO_TRX_BUSY:
			return TRXBusy;
		case RADIO_BAT_LOW:
			return batLow;
		case RADIO_BAT_OK:
			return batOK;
		case RADIO_CRC_FAILED:
			return CRCFailed;
		case RADIO_CHANNEL_ACCESS_FAILURE:
			return channelAccessFail;
		case RADIO_NO_ACK:
			return noAck;
		default:
//			assert(0);
			return unKnown;
	};
}
