/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/
/*
 * Modified to adapt to CY7C68013A by Ein Terakawa
 */

#include <fx2lib.h>
#include <fx2regs.h>
#include "DAP.h"
#include "IO_Config.h"

extern uint8_t data[4];

#define PIN_SWCLK_CLR() \
  PIN_SWCLK = 0

#define PIN_SWCLK_SET() \
  PIN_SWCLK = 1

#define PIN_SWDIO_OUT(bit) \
  PIN_SWDIO = (bit & 1)

#define PIN_SWDIO_IN() \
  PIN_SWDIO

#define PIN_DELAY() \
  do { } while(0)

#define SW_CLOCK_CYCLE()                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_WRITE_BIT(bit)               \
  PIN_SWDIO_OUT(bit);                   \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_READ_BIT(bit)                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  bit = PIN_SWDIO_IN();                 \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define PIN_SWDIO_OUT_DISABLE() \
  PIN_SWDIO_OE &= ~PIN_SWDIO_MASK

#define PIN_SWDIO_OUT_ENABLE() \
  PIN_SWDIO_OE |= PIN_SWDIO_MASK

void SWD_Init() {
  // Configure pins
  PIN_SWDIO = 1;      // set SWDIO to high
  PIN_SWCLK = 1;      // set SWCLK to high
  PIN_SWDIO_OE |= PIN_SWDIO_MASK;
  PIN_SWCLK_OE |= PIN_SWCLK_MASK;
}

void SWD_WriteSequence(uint8_t num, __xdata const uint8_t *ptr) {
  PIN_SWDIO_OUT(*ptr);
  PIN_SWDIO_OUT_ENABLE();
  // num == 0 indicates 256bits
  if ((num & 7) != 0) {
    num = (num ^ 7) + 9;
  }
  do {
    uint8_t bits = *ptr++;
    num -= 8;
    uint8_t n = ((num & 0xF8) != 0) ? 8 : 8 - num;
    for (uint8_t i=0; i < n; i++) {
      SW_WRITE_BIT(bits);
      bits >>= 1;
    }
  } while((num & 0xF8) != 0);
}

void SWD_ReadSequence(uint8_t num, __xdata uint8_t *ptr) {
  if (num == 0)
    return;

  PIN_SWDIO_OUT_DISABLE();
  do {
    uint8_t bits = 0;
    uint8_t bit = 1;
    uint8_t n = ((num < 8) ? num : 8);
    for (uint8_t i=0; i < n; i++) {
      PIN_SWCLK = 0;      // set SWCLK to low
      if (PIN_SWDIO) {
        bits |= bit;
      }
      PIN_SWCLK = 1;      // set SWCLK to high
      bit += bit;
    }
    *ptr++ = bits;
    num -= n;
  } while(num != 0);
}


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t SWD_Transfer (uint8_t request /* , uint32_t *data */) {
  uint8_t ack;
  uint8_t bit;
  uint8_t val;
  uint8_t parity;

  uint8_t m, n;

  /* Packet Request */
  PIN_SWDIO_OUT(1);
  PIN_SWDIO_OUT_ENABLE();
  SW_CLOCK_CYCLE();                     /* Start Bit */
  parity = 0;
  bit = request >> 0;
  SW_WRITE_BIT(bit);                    /* APnDP Bit */
  parity += bit;
  bit = request >> 1;
  SW_WRITE_BIT(bit);                    /* RnW Bit */
  parity += bit;
  bit = request >> 2;
  SW_WRITE_BIT(bit);                    /* A2 Bit */
  parity += bit;
  bit = request >> 3;
  SW_WRITE_BIT(bit);                    /* A3 Bit */
  parity += bit;
  SW_WRITE_BIT(parity);                 /* Parity Bit */
  SW_WRITE_BIT(0);                      /* Stop Bit */
  SW_WRITE_BIT(1);                      /* Park Bit */

  /* Turnaround */
  PIN_SWDIO_OUT_DISABLE();
  for (n = DAP_Data.swd_conf.turnaround; n; n--) {
    SW_CLOCK_CYCLE();
  }

  /* Acknowledge response */
  SW_READ_BIT(bit);
  ack  = bit << 0;
  SW_READ_BIT(bit);
  ack |= bit << 1;
  SW_READ_BIT(bit);
  ack |= bit << 2;

  if (ack == DAP_TRANSFER_OK) {         /* OK response */
    /* Data transfer */
    if (request & DAP_TRANSFER_RnW) {
      /* Read data */
      parity = 0;
      for (m = 0; m < 4; m++) {
        val = 0;
        for (n = 0; n < 8; n++) {
          SW_READ_BIT(bit);               /* Read RDATA[0:31] */
          val |= bit;
          val = (val >> 1) | (val << 7);
        }
        // 8051 trick here, to calculate parity
        ACC = val;
        parity += P;
        data[m] = val;
      }
      SW_READ_BIT(bit);                 /* Read Parity */
      if ((parity + bit) & 1) {
        ack = DAP_TRANSFER_ERROR;
      }
      // if (data) { *data = val; }
      /* Turnaround */
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {
        SW_CLOCK_CYCLE();
      }
      PIN_SWDIO_OUT_ENABLE();
    } else {
      /* Turnaround */
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {
        SW_CLOCK_CYCLE();
      }
      PIN_SWDIO_OUT_ENABLE();
      /* Write data */
      // val = *data;
      parity = 0;
      for (m = 0; m < 4; m++) {
        val = data[m];
        // 8051 trick here, to calculate parity
        ACC = val;
        parity += P;
        for (n = 0; n < 8; n++) {
          SW_WRITE_BIT(val);              /* Write WDATA[0:31] */
          val = (val >> 1) | (val << 7);
        }
      }
      SW_WRITE_BIT(parity);             /* Write Parity Bit */
    }
#if 0
    /* Capture Timestamp */
    if (request & DAP_TRANSFER_TIMESTAMP) {
      DAP_Data.timestamp = TIMESTAMP_GET();
    }
#endif
    /* Idle cycles */
    n = DAP_Data.transfer.idle_cycles;
    if (n) {
      PIN_SWDIO_OUT(0);
      for (; n; n--) {
        SW_CLOCK_CYCLE();
      }
    }
    PIN_SWDIO_OUT(1);
    return ((uint8_t)ack);
  }

  if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {
    /* WAIT or FAULT response */
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0)) {
      for (n = 32 + 1; n; n--) {
        SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */
      }
    }
    /* Turnaround */
    for (n = DAP_Data.swd_conf.turnaround; n; n--) {
      SW_CLOCK_CYCLE();
    }
    PIN_SWDIO_OUT_ENABLE();
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0)) {
      PIN_SWDIO_OUT(0);
      for (n = 32 + 1; n; n--) {
        SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */
      }
    }
    PIN_SWDIO_OUT(1);
    return ((uint8_t)ack);
  }

  /* Protocol error */
  for (n = DAP_Data.swd_conf.turnaround + 32 + 1; n; n--) {
    SW_CLOCK_CYCLE();                   /* Back off data phase */
  }
  PIN_SWDIO_OUT_ENABLE();
  PIN_SWDIO_OUT(1);
  return ((uint8_t)ack);
}
