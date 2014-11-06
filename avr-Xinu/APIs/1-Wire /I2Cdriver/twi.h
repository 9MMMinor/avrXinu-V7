//
//  twi.h
//  TWIDriver
//
//  Created by Michael Minor on 12/28/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

#ifndef TWIDriver_twi_h
#define TWIDriver_twi_h

#include <inttypes.h>

#ifndef CPU_FREQ
#define CPU_FREQ 8000000L
#endif

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MTX   2
#define TWI_SRX   3
#define TWI_STX   4


struct twi_Command {
	uint8_t slarw;
	uint8_t *data;
	uint8_t dataLength;
	struct twi_Command *chain;
};

// twi prototypes
void twi_init(void);
int twi_doCommand( struct twi_Command * );

// twi globals
extern volatile uint8_t twi_internal_state;

/*
 *-----------------------------------------------------------------------------------
 * This macro inserts a pointer to a twi_Command Block
 * in the chain entry of \c cmd_blk, allowing chained twi operations.
 *-----------------------------------------------------------------------------------
 */
#define twi_setChain(cmd_blk, new_chain) do { (cmd_blk)->chain = new_chain; } while(0)

/*
 *-----------------------------------------------------------------------------------
 * This macro retrieves the chain entry from \c cmd_blk.
 *-----------------------------------------------------------------------------------
 */
#define twi_getChain(cmd_blk) ((cmd_blk)->chain)

/*
 *-----------------------------------------------------------------------------------
 * Setup a user-supplied buffer as a twi-Command Block
 * 
 * This macro takes a user-supplied buffer \c cmd_blk, and sets it up
 * as a Command Block that is valid for TWI operations.
 * The buffer to setup must be of type (struct twi_Command *).
 * 
 * The argument \c tw_address is the Two-Wire (I2C) device address
 * 
 * The \c tw_rw argument can take one of the values TW_READ or
 * TW_WRITE, for read or write operation on the Two-Wire bus.
 * 
 * The \c tw_data argument is a pointer to a uint8_t buffer, and
 * \c tw_len is the length of that buffer.
 *-----------------------------------------------------------------------------------
 */
#define twi_setupCmdBlock(cmd_blk, tw_address, tw_rw, tw_data, tw_len, tw_chain) \
do { \
(cmd_blk)->slarw = (tw_address<<1|tw_rw); \
(cmd_blk)->data = (tw_data); \
(cmd_blk)->dataLength = (tw_len); \
(cmd_blk)->chain = (tw_chain); \
} while(0)

/*
 *-----------------------------------------------------------------------------------
 * Initializer for a user-supplied twi-Command Block
 * 
 * This macro acts similar to twi_setupCmdBlock(), but it is to be
 * used as the initializer of a variable of (struct twi_Command *).
 * 
 * The remaining arguments are to be used as explained in
 * twi_setupCmdBlk().
 *-----------------------------------------------------------------------------------
 */
#define TWI_SETUP_CMDBLOCK(tw_address, tw_rw, tw_data, tw_len, tw_chain) \
{ \
.slarw = (tw_address<<1|tw_rw), \
.data = tw_data, \
.dataLength = tw_len, \
.chain = tw_chain, \
}

#endif
