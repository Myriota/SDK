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

/// @defgroup BSP_api BSP API
/// Board specific implementation of the board support package (BSP).
/// <a
/// href="https://github.com/Myriota/SDK/blob/master/terminal/g2/boards/MyriotaDB/bsp.c">Myriota
/// development board BSP</a>
///  is used by default.
/// Create bsp.c under application folder to override default BSP file
/// @{

/// Compile time check
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

/// @defgroup Board_env Board-specific environment
/// @{

/// Get board-specific environment variables
/// Returns string which contains semicolon separated variables.
/// - GNSSFIX
///     - 0: skip system GNSS fix
///     - 1: get time only after location has been fixed or set
///     - unset: do system GNSS fix
///     .
/// - DUMPTX
///     - 1: dump transmission information
///     - 0 or unset: don't dump
///     .
/// .
#define BOARD_ENV_LEN_MAX 50
char *BoardEnvGet(void);

/// @}

/// @defgroup Board_init Board initialisation
/// @{

/// Initialise the board before any system hardware initialisation, invoked
/// only once at startup.
/// Returns 0 if succeeded and -1 if failed.
int BoardInit(void);
/// Initialise the board after the system hardware initialisation, invoked only
/// once at startup. Can be overriden by your application code individually.
/// Returns 0 if succeeded and -1 if failed.
int BoardStart(void);

/// @}

/// @defgroup Board_battery Battery voltage reading
/// @{

/// Get the battery voltage in millivolt.
/// Returns 0 if succeeded and -1 if failed.
/// If battery measurement is not supported then both voltage and the return
/// value should be 0.
int BoardBatteryVoltGet(uint32_t *mv);

/// @}

/// @defgroup Board_led LED control
/// @{

/// Control the LED for system status indication.
void BoardLEDInit(void);
void BoardLEDDeinit(void);
void BoardLEDTurnOn(void);
void BoardLEDTurnOff(void);
void BoardLEDToggle(void);

/// @}

/// @defgroup Board_gnss GNSS module power control
/// @{

void BoardGNSSPowerEnable(void);
void BoardGNSSPowerDisable(void);
/// Returns true if enabled and false if disabled.
bool BoardGNSSPowerIsEnabled(void);

/// @}

/// @defgroup Board_radio Satellite radio antenna control
/// @{

/// Select the antenna based on satellite radio mode and frequency band.
typedef enum {
  RADIO_MODE_RX,      ///< radio downlink
  RADIO_MODE_TX,      ///< radio uplink
  RADIO_MODE_INIT,    ///< radio initialisation
  RADIO_MODE_DEINIT,  ///< radio de-initialisation
} RadioMode;
typedef enum { RADIO_BAND_VHF, RADIO_BAND_UHF, RADIO_BAND_ISM } RadioBand;
/// Returns 0 if succeeded and -1 if failed.
int BoardAntennaSelect(RadioMode Mode, RadioBand Band);

/// @}

/// @defgroup Board_debug Debug output
/// @{

void *BoardDebugInit(void);
void BoardDebugDeinit(void);
/// Returns 0 if all bytes are sent and -1 if not.
int BoardDebugWrite(const uint8_t *Tx, size_t Length);
/// Returns number of bytes read back and -1 if read fails.
int BoardDebugRead(uint8_t *Rx, size_t Length);

/// @}

/// @}

#endif  // MYRIOTA_BSP_H
