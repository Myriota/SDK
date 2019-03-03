// Copyright (c) 2016-2019, Myriota Pty Ltd, All Rights Reserved
// SPDX-License-Identifier: BSD-3-Clause-Attribution
//
// This file is licensed under the BSD with attribution  (the "License"); you
// may not use these files except in compliance with the License.
//
// You may obtain a copy of the License here:
// LICENSE-BSD-3-Clause-Attribution.txt and at
// https://spdx.org/licenses/BSD-3-Clause-Attribution.html
//
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MYRIOTA_BSP_H
#define MYRIOTA_BSP_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

// Carrier board specific implementation of the board support package

/// Board initialisation
// Board initialisation before any system hardware initialisation, invoked
// only once at startup
int BoardInit(void);
// Board initialisation after the system hardware initialisation, invoked only
// once at startup
// Can be overriden if you only want to change how the system indicates it
// has started
int BoardStart(void);

/// Battery
// Get the battery voltage in mv
// Returns 0 if succeeded and -1 if failed
// 0mv if battery measurement is not supported and returns success
int BoardBatteryVoltGet(uint32_t *mv);

/// LED
void BoardLEDInit(void);
void BoardLEDDeinit(void);
void BoardLEDTurnOn(void);
void BoardLEDTurnOff(void);
void BoardLEDToggle(void);

/// GPS
void BoardGNSSPowerEnable(void);
void BoardGNSSPowerDisable(void);
bool BoardGNSSPowerIsEnabled(void);

/// RADIO
typedef enum { RADIO_BAND_VHF, RADIO_BAND_UHF, RADIO_BAND_ISM } RadioBand;
int BoardAntennaSelect(bool IsTx, RadioBand Band);

/// UART debug
void *BoardDebugInit(void);
void BoardDebugDeinit(void);
int BoardDebugWrite(const uint8_t *Tx, size_t Length);

#endif  // MYRIOTA_BSP_H
