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
 * $Date:        7. September 2021
 * $Revision:    V2.1.1
 *
 * Project:      CMSIS-DAP Source
 * Title:        DAP.c CMSIS-DAP Commands
 *
 *---------------------------------------------------------------------------*/
/*
 * Modified to adapt to CY7C68013A by Ein Terakawa
 */

#include <fx2lib.h>
#include <fx2regs.h>
#include "DAP.h"

         DAP_Data_t DAP_Data;           // DAP Data
volatile uint8_t    DAP_TransferAbort;  // Transfer Abort Flag

uint8_t data[4];


uint16_t dap_execute_command(uint16_t idxidx, uint8_t len) {
  (void)len;
  uint8_t idx = idxidx & 0xFF;
  __xdata uint8_t *ptr = &EP2FIFOBUF[idxidx >> 8];
  uint8_t cmd = *ptr++;
  scratch[idx++] = cmd;
  if (cmd == ID_DAP_ExecuteCommands) {
    uint8_t cnt = *ptr++;
    scratch[idx++] = cnt;
    idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
    do {
      idxidx = dap_execute_command(idxidx, 0);
      cnt -= 1;
    } while (cnt != 0);
    return idxidx;
  } if (cmd == ID_DAP_Info) {
    uint8_t info = *ptr++;
    if (info == DAP_ID_DAP_FW_VER) {
      scratch[idx++] = 6;
      scratch[idx++] = '2';
      scratch[idx++] = '.';
      scratch[idx++] = '1';
      scratch[idx++] = '.';
      scratch[idx++] = '0';
      scratch[idx++] = 0;
    } else if (info == DAP_ID_SER_NUM) {
      scratch[idx++] = 17;
      scratch[idx++] = '0';
      scratch[idx++] = '0';
      scratch[idx++] = '1';
      scratch[idx++] = '1';
      scratch[idx++] = '2';
      scratch[idx++] = '2';
      scratch[idx++] = '3';
      scratch[idx++] = '3';
      scratch[idx++] = '4';
      scratch[idx++] = '4';
      scratch[idx++] = '5';
      scratch[idx++] = '5';
      scratch[idx++] = '6';
      scratch[idx++] = '6';
      scratch[idx++] = '7';
      scratch[idx++] = '7';
      scratch[idx++] = 0;
    } else if (info == DAP_ID_PRODUCT_FW_VER) {
      scratch[idx++] = 5;
      scratch[idx++] = '0';
      scratch[idx++] = '0';
      scratch[idx++] = '0';
      scratch[idx++] = '0';
      scratch[idx++] = 0;
    } else if (info == DAP_ID_CAPABILITIES) {
      scratch[idx++] = 1;
      scratch[idx++] = (1 << 0); // SWD support
    } else if (info == DAP_ID_PACKET_SIZE) {
      scratch[idx++] = 2;
      scratch[idx++] = 64;
      scratch[idx++] = 0;
    } else if (info == DAP_ID_PACKET_COUNT) {
      scratch[idx++] = 1;
      scratch[idx++] = 1;
    } else {
      scratch[idx++] = 0;
    }
  } else if (cmd == ID_DAP_HostStatus) {
    if (*ptr == 0) {
      // led_connected = ptr[1];
    }
    ptr += 2;
    scratch[idx++] = DAP_OK;
  } else if (cmd == ID_DAP_Connect) {
    ptr += 1;
    scratch[idx++] = DAP_PORT_SWD; // SWD Port
  } else if (cmd == ID_DAP_Disconnect) {
    scratch[idx++] = DAP_OK;
  } else if (cmd == ID_DAP_TransferConfigure) {
    ptr += 5;
    scratch[idx++] = DAP_OK;
  } else if (cmd == ID_DAP_Transfer) {
    idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
    return dap_execute_transfer(idxidx, len);
  } else if (cmd == ID_DAP_TransferBlock) {
    idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
    return dap_execute_transfer_block(idxidx, len);
  } else if (cmd == ID_DAP_SWJ_Clock) {
    ptr += 4;
    scratch[idx++] = DAP_OK;
  } else if (cmd == ID_DAP_SWD_Configure) {
    ptr += 1;
    // Configure pins
    SWD_Init();
    scratch[idx++] = DAP_OK;
  } else if (cmd == ID_DAP_SWJ_Sequence) {
    idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
    return dap_execute_swj_sequence(idxidx, len);
  } else if (cmd == ID_DAP_SWD_Sequence) {
    idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
    return dap_execute_swd_sequence(idxidx, len);
  } else {
    scratch[idx++] = DAP_ERROR;
  }
  idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
  return idxidx;
}

uint16_t dap_execute_swj_sequence(uint16_t idxidx, uint8_t len) {
  (void)len;
  uint8_t idx = idxidx & 0xFF;
  __xdata uint8_t *ptr = &EP2FIFOBUF[idxidx >> 8];

  // led_blink();

  uint8_t num = *ptr++;
  // num == 0 indicates 256bits
  SWD_WriteSequence(num, ptr);
  ptr += (uint8_t)(((uint8_t)(num - 1) >> 3) + 1);
  scratch[idx++] = DAP_OK;
  idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
  return idxidx;
}

uint16_t dap_execute_swd_sequence(uint16_t idxidx, uint8_t len) {
  (void)len;
  uint8_t idx = idxidx & 0xFF;
  __xdata uint8_t *ptr = &EP2FIFOBUF[idxidx >> 8];

  // led_blink();

  scratch[idx++] = DAP_OK;
  uint8_t count = *ptr++;
  while (count--) {
    uint8_t num = *ptr++;
    if ((num & 0x80) == 0) {
      num &= 0x3F;
      // num == 0 indicates 64bits
      if (num == 0) {
        num = 64;
      }
      SWD_WriteSequence(num, ptr);
      ptr += ((uint8_t)(num + 7) >> 3);
    } else {
      num &= 0x3F;
      // num == 0 indicates 64bits
      if (num == 0) {
        num = 64;
      }
      SWD_ReadSequence(num, &scratch[idx]);
      idx += ((uint8_t)(num + 7) >> 3);
    }
  }

  idxidx = (uint16_t)idx | (uint16_t)((uint16_t)ptr & 0xFF) << 8;
  return idxidx;
}

uint16_t dap_execute_transfer(uint16_t idxidx, uint8_t len) {
  (void)len;
  __xdata uint8_t *request = &EP2FIFOBUF[idxidx >> 8];
  uint8_t idx = idxidx & 0xFF;
  __xdata uint8_t *response = &scratch[idx];

  // uint8_t dap_port = *ptr++;
  // uint8_t cnt = *ptr++;

  // static uint32_t DAP_SWD_Transfer(const uint8_t *request, uint8_t *response) {
  // const
  // uint8_t  *request_head;
  uint8_t  request_count;
  uint8_t  request_value;
  __xdata uint8_t *response_head;
  uint8_t  response_count;
  uint8_t  response_value;
  uint8_t  post_read;
  uint8_t  check_write;
  // uint32_t  match_value;
  uint8_t  match_value[4];
  uint8_t  match_retry;
  uint8_t  retry;
  uint8_t  match;
  // uint32_t  data;

  // request_head   = request;

  response_count = 0U;
  response_value = 0U;
  response_head  = response;
  response      += 2;

  DAP_TransferAbort = 0U;

  post_read   = 0U;
  check_write = 0U;

  request++;            // Ignore DAP index

  request_count = *request++;

  for (; request_count != 0U; request_count--) {
    request_value = *request++;
    if ((request_value & DAP_TRANSFER_RnW) != 0U) {
      // Read register
      if (post_read) {
        // Read was posted before
        retry = DAP_Data.transfer.retry_count;
        if ((request_value & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP) {
          // Read previous AP data and post next AP read
          do {
            response_value = SWD_Transfer(request_value /*, &data */);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        } else {
          // Read previous AP data
          do {
            response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW /*, &data */);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          post_read = 0U;
        }
        if (response_value != DAP_TRANSFER_OK) {
          break;
        }
        // Store previous AP data
        // *response++ = (uint8_t) data;
        // *response++ = (uint8_t)(data >>  8);
        // *response++ = (uint8_t)(data >> 16);
        // *response++ = (uint8_t)(data >> 24);
        *response++ = data[0];
        *response++ = data[1];
        *response++ = data[2];
        *response++ = data[3];
      }
      if ((request_value & DAP_TRANSFER_MATCH_VALUE) != 0U) {
        // Read with value match
        // match_value = (uint32_t)(*(request+0) <<  0) |
        //               (uint32_t)(*(request+1) <<  8) |
        //               (uint32_t)(*(request+2) << 16) |
        //               (uint32_t)(*(request+3) << 24);
        // request += 4;
        match_value[0] = *request++;
        match_value[1] = *request++;
        match_value[2] = *request++;
        match_value[3] = *request++;
        match_retry = DAP_Data.transfer.match_retry;
        if ((request_value & DAP_TRANSFER_APnDP) != 0U) {
          // Post AP read
          retry = DAP_Data.transfer.retry_count;
          do {
            response_value = SWD_Transfer(request_value /*, NULL */);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          if (response_value != DAP_TRANSFER_OK) {
            break;
          }
        }
        match = 0;
        do {
          // Read register until its value matches or retry counter expires
          retry = DAP_Data.transfer.retry_count;
          do {
            response_value = SWD_Transfer(request_value /*, &data */);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          if (response_value != DAP_TRANSFER_OK) {
            break;
          }
          match = 1;
          for (uint8_t i = 0; i < 4; i++) {
	    if ((data[i] & DAP_Data.transfer.match_mask[i]) != match_value[i]) {
              match = 0;
              break;
            }
	  }
        } while (!match && match_retry-- && !DAP_TransferAbort);
        // } while (((data & DAP_Data.transfer.match_mask) != match_value) && match_retry-- && !DAP_TransferAbort);
        // if ((data & DAP_Data.transfer.match_mask) != match_value) {
        if (!match) {
          response_value |= DAP_TRANSFER_MISMATCH;
        }
        if (response_value != DAP_TRANSFER_OK) {
          break;
        }
      } else {
        // Normal read
        retry = DAP_Data.transfer.retry_count;
        if ((request_value & DAP_TRANSFER_APnDP) != 0U) {
          // Read AP register
          if (post_read == 0U) {
            // Post AP read
            do {
              response_value = SWD_Transfer(request_value /*, NULL */);
            } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
            if (response_value != DAP_TRANSFER_OK) {
              break;
            }
            post_read = 1U;
          }
        } else {
          // Read DP register
          do {
            response_value = SWD_Transfer(request_value /*, &data */);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          if (response_value != DAP_TRANSFER_OK) {
            break;
          }
          // Store data
          // *response++ = (uint8_t) data;
          // *response++ = (uint8_t)(data >>  8);
          // *response++ = (uint8_t)(data >> 16);
          // *response++ = (uint8_t)(data >> 24);
          *response++ = data[0];
          *response++ = data[1];
          *response++ = data[2];
          *response++ = data[3];
        }
      }
      check_write = 0U;
    } else {
      // Write register
      if (post_read) {
        // Read previous data
        retry = DAP_Data.transfer.retry_count;
        do {
          response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW /*, &data */);
        } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        if (response_value != DAP_TRANSFER_OK) {
          break;
        }
        // Store previous data
        // *response++ = (uint8_t) data;
        // *response++ = (uint8_t)(data >>  8);
        // *response++ = (uint8_t)(data >> 16);
        // *response++ = (uint8_t)(data >> 24);
        *response++ = data[0];
        *response++ = data[1];
        *response++ = data[2];
        *response++ = data[3];
        post_read = 0U;
      }
      // Load data
      // data = (uint32_t)(*(request+0) <<  0) |
      //        (uint32_t)(*(request+1) <<  8) |
      //        (uint32_t)(*(request+2) << 16) |
      //        (uint32_t)(*(request+3) << 24);
      // request += 4;
      data[0] = *request++;
      data[1] = *request++;
      data[2] = *request++;
      data[3] = *request++;
      if ((request_value & DAP_TRANSFER_MATCH_MASK) != 0U) {
        // Write match mask
        // DAP_Data.transfer.match_mask = data;
	DAP_Data.transfer.match_mask[0] = data[0];
	DAP_Data.transfer.match_mask[1] = data[1];
	DAP_Data.transfer.match_mask[2] = data[2];
	DAP_Data.transfer.match_mask[3] = data[3];
        response_value = DAP_TRANSFER_OK;
      } else {
        // Write DP/AP register
        retry = DAP_Data.transfer.retry_count;
        do {
          response_value = SWD_Transfer(request_value /*, &data */);
        } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        if (response_value != DAP_TRANSFER_OK) {
          break;
        }
        check_write = 1U;
      }
    }
    response_count++;
    if (DAP_TransferAbort) {
      break;
    }
  }

  for (; request_count != 0U; request_count--) {
    // Process canceled requests
    request_value = *request++;
    if ((request_value & DAP_TRANSFER_RnW) != 0U) {
      // Read register
      if ((request_value & DAP_TRANSFER_MATCH_VALUE) != 0U) {
        // Read with value match
        request += 4;
      }
    } else {
      // Write register
      request += 4;
    }
  }

  if (response_value == DAP_TRANSFER_OK) {
    if (post_read) {
      // Read previous data
      retry = DAP_Data.transfer.retry_count;
      do {
        response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW /*, &data */);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) {
        goto end;
      }
      // Store previous data
      // *response++ = (uint8_t) data;
      // *response++ = (uint8_t)(data >>  8);
      // *response++ = (uint8_t)(data >> 16);
      // *response++ = (uint8_t)(data >> 24);
      *response++ = data[0];
      *response++ = data[1];
      *response++ = data[2];
      *response++ = data[3];
    } else if (check_write) {
      // Check last write
      retry = DAP_Data.transfer.retry_count;
      do {
        response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW /*, NULL */);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
    }
  }

end:
  *(response_head+0) = (uint8_t)response_count;
  *(response_head+1) = (uint8_t)response_value;

  // return (((uint32_t)(request - request_head) << 16) | (uint32_t)(response - response_head));
  // }

  idxidx = (uint16_t)((uint16_t)response & 0xFF) | (uint16_t)((uint16_t)request & 0xFF) << 8;
  return idxidx;
}

uint16_t dap_execute_transfer_block(uint16_t idxidx, uint8_t len) {
  (void)len;
  __xdata uint8_t *request = &EP2FIFOBUF[idxidx >> 8];
  uint8_t idx = idxidx & 0xFF;
  __xdata uint8_t *response = &scratch[idx];

  // static uint32_t DAP_SWD_TransferBlock(const uint8_t *request, uint8_t *response) {

  uint16_t  request_count;
  uint8_t  request_value;
  uint16_t  response_count;
  uint8_t  response_value;
  __xdata uint8_t  *response_head;
  uint8_t  retry;
  // uint32_t  data;

  response_count = 0U;
  response_value = 0U;
  response_head  = response;
  response      += 3;

  DAP_TransferAbort = 0U;

  request++;            // Ignore DAP index

  request_count = ((uint16_t)*(request+0) << 0) |
                  ((uint16_t)*(request+1) << 8);
  request += 2;
  if (request_count == 0U) {
    goto end;
  }

  request_value = *request++;
  if ((request_value & DAP_TRANSFER_RnW) != 0U) {
    // Read register block
    if ((request_value & DAP_TRANSFER_APnDP) != 0U) {
      // Post AP read
      retry = DAP_Data.transfer.retry_count;
      do {
        response_value = SWD_Transfer(request_value /*, NULL */);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) {
        goto end;
      }
    }
    while (request_count--) {
      // Read DP/AP register
      if ((request_count == 0U) && ((request_value & DAP_TRANSFER_APnDP) != 0U)) {
        // Last AP read
        request_value = DP_RDBUFF | DAP_TRANSFER_RnW;
      }
      retry = DAP_Data.transfer.retry_count;
      do {
        response_value = SWD_Transfer(request_value /*, &data */);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) {
        goto end;
      }
      // Store data
      // *response++ = (uint8_t) data;
      // *response++ = (uint8_t)(data >>  8);
      // *response++ = (uint8_t)(data >> 16);
      // *response++ = (uint8_t)(data >> 24);
      response_count++;
      *response++ = data[0];
      *response++ = data[1];
      *response++ = data[2];
      *response++ = data[3];
    }
  } else {
    // Write register block
    while (request_count--) {
      // Load data
      // data = (uint32_t)(*(request+0) <<  0) |
      //        (uint32_t)(*(request+1) <<  8) |
      //        (uint32_t)(*(request+2) << 16) |
      //        (uint32_t)(*(request+3) << 24);
      // request += 4;
      data[0] = *request++;
      data[1] = *request++;
      data[2] = *request++;
      data[3] = *request++;
      // Write DP/AP register
      retry = DAP_Data.transfer.retry_count;
      do {
        response_value = SWD_Transfer(request_value /*, &data */);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) {
        goto end;
      }
      response_count++;
    }
    // Check last write
    retry = DAP_Data.transfer.retry_count;
    do {
      response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW /*, NULL */);
    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
  }

end:
  *(response_head+0) = (uint8_t)(response_count >> 0);
  *(response_head+1) = (uint8_t)(response_count >> 8);
  *(response_head+2) = (uint8_t) response_value;

  // return ((uint32_t)(response - response_head));
  idxidx = (uint16_t)((uint16_t)response & 0xFF) | (uint16_t)((uint16_t)request & 0xFF) << 8;
  return idxidx;
}
