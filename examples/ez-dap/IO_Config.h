/**
 * @file    IO_Config.h
 * @brief   
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Modified to adapt to CY7C68013A by Ein Terakawa
 */

#ifndef __IO_CONFIG_H__
#define __IO_CONFIG_H__

// SWCLK Pin                            PB7
#define PIN_SWCLK PB7
#define PIN_SWCLK_OE OEB
#define PIN_SWCLK_MASK (1 << 7)

// SWDIO Pin                            PB5
#define PIN_SWDIO PB5
#define PIN_SWDIO_OE OEB
#define PIN_SWDIO_MASK (1 << 5)

// LED0 Pin                             PA0
#define PIN_LED0 PA0
#define PIN_LED0_OE OEA
#define PIN_LED0_MASK (1 << 0)

// LED1 Pin                             PA1
#define PIN_LED1 PA1
#define PIN_LED1_OE OEA
#define PIN_LED1_MASK (1 << 1)

#endif
