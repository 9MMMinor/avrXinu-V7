
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ENC28J60_PACKET_H
#define ENC28J60_PACKET_H

#include <stdbool.h>
#include <stdint.h>

/**
 * \addtogroup net
 *
 * @{
 */
/**
 * \addtogroup net_driver
 *
 * @{
 */
/**
 * \addtogroup net_driver_enc28j60
 *
 * @{
 */
/**
 * \file
 * Microchip ENC28J60 packet handling header (license: GPLv2)
 *
 * \author Roland Riegel
 */

DEVCALL enc28j60_receive_packet(uint8_t* buffer, uint16_t buffer_len);
DEVCALL enc28j60_send_packet(uint8_t* buffer, uint16_t buffer_len);

/**
 * @}
 * @}
 * @}
 */

#endif

