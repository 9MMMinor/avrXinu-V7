/*
 *  external_interrupt.h
 *  
 *
 *  Created by Michael Minor on 6/1/10.
 *  Copyright 2010 Michael M Minor. All rights reserved.
 *
 */

/* Vectors */
/* INT2_vect   External Interrupt Request 2 */

/* Registers */
/* EICRA – External Interrupt Control Register A */
/* EIMSK – External Interrupt Mask Register */
/* EIFR – External Interrupt Flag Register */
/* PCICR – Pin Change Interrupt Control Register */
/* PCIFR – Pin Change Interrupt Flag Register */
/* PCMSK3 – Pin Change Mask Register 3 */
/* PCMSK2 – Pin Change Mask Register 2 */
/* PCMSK1 – Pin Change Mask Register 1 */
/* PCMSK0 – Pin Change Mask Register 0 */

/* External Interrupt register set */
typedef struct ext_csr	{
	volatile unsigned char PCIFR_reg;			/* Pin Change Interrupt Flag Reg */
	volatile unsigned char PCICR_reg;			/* Pin Change Interrupt Control Reg */
	volatile unsigned char EIFR_reg;			/* External Interrupt Flag Register */
	volatile unsigned char EIMSK_reg;			/* External Interrupt Mask Register */
}EXT_t;

#define EXT (*(EXT_t *) 0x1B)				/* EXT.EIMSK_reg for example		*/
#define EXT_ptr ((EXT_t *) 0x1B)			/* EXT_ptr->EIFR_reg for example	*/
