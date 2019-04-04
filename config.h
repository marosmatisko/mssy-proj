/**
 * \file config.h
 *
 * \brief Peer2Peer application and stack configuration
 *
 * Copyright (C) 2012-2014, Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * Modification and other use of this code is subject to Atmel's Limited
 * License Agreement (license.txt).
 *
 * $Id: config.h 9267 2014-03-18 21:46:19Z ataradov $
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/*- Definitions ------------------------------------------------------------*/

#define UART_SPEED 115200


// Address must be set to 0 for the first device, and to 1 for the second one.
#define APP_ADDR                  1
#define APP_PANID                 0x80
#define NETWORK_ENDPOINT              1
#define GET_SET_ENDPOINT              2
#define PERIODIC_ENDPOINT             3
#define ALARM_ENDPOINT                4
//#define APP_SECURITY_KEY          "mssy2019"
#define APP_FLUSH_TIMER_INTERVAL  20

#ifdef PHY_AT86RF212
  #define APP_CHANNEL             0x01
  #define APP_BAND                0x00
  #define APP_MODULATION          0x24
#else
  #define APP_CHANNEL             0x0f
#endif

#define HAL_UART_CHANNEL          1
#define HAL_UART_RX_FIFO_SIZE     400
#define HAL_UART_TX_FIFO_SIZE     400

#define SYS_SECURITY_MODE                   0

#define NWK_BUFFERS_AMOUNT                  30
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   2000 // ms

#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#endif // _CONFIG_H_
