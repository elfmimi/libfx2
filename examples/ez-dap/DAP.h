/*
 * Copyright (c) 2013-2021 ARM Limited. All rights reserved.
 * Copyright 2019, Cypress Semiconductor Corporation
 * or a subsidiary of Cypress Semiconductor Corporation.
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
 * $Date:        26. May 2021
 * $Revision:    V2.1.0
 *
 * Project:      CMSIS-DAP Include
 * Title:        DAP.h Definitions
 *
 *---------------------------------------------------------------------------*/
/*
 * Modified to adapt to CY7C68013A by Ein Terakawa
 */

#ifndef __DAP_H__
#define __DAP_H__

// DAP Firmware Version
#define DAP_FW_VER                      "2.1.0"


// DAP Command IDs
#define ID_DAP_Info                     0x00U
#define ID_DAP_HostStatus               0x01U
#define ID_DAP_Connect                  0x02U
#define ID_DAP_Disconnect               0x03U
#define ID_DAP_TransferConfigure        0x04U
#define ID_DAP_Transfer                 0x05U
#define ID_DAP_TransferBlock            0x06U
#define ID_DAP_SWJ_Clock                0x11U
#define ID_DAP_SWJ_Sequence             0x12U
#define ID_DAP_SWD_Configure            0x13U
#define ID_DAP_SWD_Sequence             0x1DU

#define ID_DAP_ExecuteCommands          0x7FU

// DAP Status Code
#define DAP_OK                          0U
#define DAP_ERROR                       0xFFU

// DAP ID
#define DAP_ID_VENDOR                   1U
#define DAP_ID_PRODUCT                  2U
#define DAP_ID_SER_NUM                  3U
#define DAP_ID_DAP_FW_VER               4U
#define DAP_ID_PRODUCT_FW_VER           9U
#define DAP_ID_CAPABILITIES             0xF0U
#define DAP_ID_PACKET_COUNT             0xFEU
#define DAP_ID_PACKET_SIZE              0xFFU

// DAP Port
#define DAP_PORT_SWD                    1U      // SWD Port (SWCLK, SWDIO) + nRESET

// DAP Transfer Request
#define DAP_TRANSFER_APnDP              (1U<<0)
#define DAP_TRANSFER_RnW                (1U<<1)
#define DAP_TRANSFER_A2                 (1U<<2)
#define DAP_TRANSFER_A3                 (1U<<3)
#define DAP_TRANSFER_MATCH_VALUE        (1U<<4)
#define DAP_TRANSFER_MATCH_MASK         (1U<<5)

// DAP Transfer Response
#define DAP_TRANSFER_OK                 (1U<<0)
#define DAP_TRANSFER_WAIT               (1U<<1)
#define DAP_TRANSFER_FAULT              (1U<<2)
#define DAP_TRANSFER_ERROR              (1U<<3)
#define DAP_TRANSFER_MISMATCH           (1U<<4)

// Debug Port Register Addresses
#define DP_RDBUFF                       0x0CU   // Read Buffer (Read Only)

// DAP Data structure
typedef struct {
  struct {
    uint8_t retry_count;
    uint8_t idle_cycles;
    uint8_t match_retry;
    uint8_t match_mask[4];
  } transfer;
  struct {
    uint8_t turnaround;
    uint8_t data_phase;
  } swd_conf;
} DAP_Data_t;

extern          DAP_Data_t DAP_Data;            // DAP Data
extern volatile uint8_t    DAP_TransferAbort;   // Transfer Abort Flag

#ifdef  __cplusplus
extern "C"
{
#endif

// Functions
extern void SWD_Init();
extern void SWD_WriteSequence(uint8_t num, __xdata const uint8_t *ptr);
extern void SWD_ReadSequence(uint8_t num, __xdata uint8_t *ptr);
extern uint8_t SWD_Transfer(uint8_t request /* , uint32_t *data */);

extern uint16_t dap_execute_command(uint16_t idxidx, uint8_t len);
extern uint16_t dap_execute_swj_sequence(uint16_t idxidx, uint8_t len);
extern uint16_t dap_execute_swd_sequence(uint16_t idxidx, uint8_t len);
extern uint16_t dap_execute_transfer(uint16_t idxidx, uint8_t len);
extern uint16_t dap_execute_transfer_block(uint16_t idxidx, uint8_t len);

extern void led_blink();

#ifdef  __cplusplus
}
#endif

#endif  /* __DAP_H__ */
