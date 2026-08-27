// Copyright (c) 2016-2022, Myriota Pty Ltd, All Rights Reserved
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
/// \brief Board specific implementation of the board support package (BSP).
/// \details <a href="https://github.com/Myriota/SDK/blob/master/module/g2/
/// boards/MyriotaDB/bsp.c">Myriota development board BSP</a> is used by
/// default. Create  a `bsp.c` under application folder to override default BSP
/// file.
/// @{

/// Compile time check
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

/// @defgroup Board_env Board-specific environment
/// \brief Board configuration and environment variable access.
/// @{

/// \brief Environment variable string length limit.
#define BOARD_ENV_LEN_MAX 50

/// \brief Get board-specific environment variables
/// \details Returns string which contains semicolon separated variables.
/// - GNSSFIX
///     - 0: skip system GNSS fix and initial location is invalid
///     - 1: get time only after location has been fixed or set
///     - 2: get time and do GNSS 2D fix
///     - 3: get time and do GNSS 3D fix; this is default mode
///     - 4: skip system GNSS fix and initial location is set to 0,0
///     - unset: the same as 3
///     .
/// - DUMPTX (Deprecated, do not use)
///     - 1: dump transmission information
///     - 0 or unset: don't dump
///     .
/// - SATTEST (supersede DUMPTX)
///     - 1~255: Enable satellite communication test, including dumping
///     transmission information and communication stats, and specify
///     communication stats dumping interval in minute. The dumped data is
///     logged using user logging code 255
///     - 0 or unset: disabled
///     .
/// .
/// \deprecated DUMPTX is deprecated and should not be used. Use SATTEST
/// instead.
char *BoardEnvGet(void);

/// @}

/// @defgroup Board_init Board initialisation
/// \brief Board initialisation and startup sequencing.
/// @{

/// \brief Initialise the board before any system hardware initialisation.
/// \details This function is invoked only once at startup.
/// \return 0 if succeeded, -1 if failed.
int BoardInit(void);
/// \brief Initialise the board after the system hardware initialisation.
/// \details This function is invoked only once at startup. Can be overridden by
/// your application code individually.
/// \return 0 if succeeded, -1 if failed.
int BoardStart(void);

/// @}

/// @defgroup Board_battery Battery voltage reading
/// \brief Battery voltage measurement for power monitoring.
/// @{

/// \brief Get the battery voltage in millivolt.
/// \param mv[out] Pointer to store the battery voltage in millivolt.
/// \return 0 if succeeded and -1 if failed.
/// \note If battery measurement is not supported then both voltage and the
/// return value are 0.
int BoardBatteryVoltGet(uint32_t *mv);

/// @}

/// @defgroup Board_led LED control
/// Status LED control for system indication.
/// @{

/// \brief Initialise the status LED.
/// \details Configures GPIO and peripheral settings for the status LED. Must
/// be called before using BoardLEDTurnOn(), BoardLEDTurnOff(), or
/// BoardLEDToggle().
void BoardLEDInit(void);
/// \brief Deinitialise the status LED.
/// \details Disables the LED and releases associated GPIO resources.
void BoardLEDDeinit(void);
/// \brief Turn on the status LED.
/// \details Powers on the status LED.
void BoardLEDTurnOn(void);
/// \brief Turn off the status LED.
/// \details Powers off the status LED.
void BoardLEDTurnOff(void);
/// \brief Toggle the status LED state.
/// \details Switches the LED to the opposite state: if on, turns it off; if
/// off, turns it on. Useful for visual feedback without tracking the LED state
/// manually.
void BoardLEDToggle(void);

/// @}

/// @defgroup Board_gnss GNSS module power control
/// \brief GNSS receiver power management and position acquisition.
/// \details For more details about supported GNSS modules, see the <a
/// href="https://support.myriota.com/hc">Myriota support site</a> or contact
/// our <a href="https://support.myriota.com/hc/requests/new">Support Team</a>.
/// @{

/// \brief Enable power to the GNSS module.
/// \details This is useful for relaying GNSS message to the user application.
/// Power consumption increases while the GNSS module is active.
void BoardGNSSPowerEnable(void);
/// \brief Disable power to the GNSS module.
/// \details  Powers off the GNSS receiver to save power when GNSS is not
/// needed.
void BoardGNSSPowerDisable(void);
/// \brief Check if the GNSS module is powered on.
/// \return True if GNSS module is powered, false if powered off.
/// \see BoardGNSSPowerEnable(), BoardGNSSPowerDisable(), HasValidGNSSFix()
bool BoardGNSSPowerIsEnabled(void);

/// @}

/// @defgroup Board_radio Satellite radio antenna control
/// \brief Antenna selection and RF configuration.
/// @{

/// \brief Select the antenna based on satellite radio mode and frequency band.
typedef enum {
  RADIO_MODE_RX,      ///< Radio downlink.
  RADIO_MODE_TX,      ///< Radio uplink.
  RADIO_MODE_INIT,    ///< Radio initialisation.
  RADIO_MODE_DEINIT,  ///< Radio de-initialisation.
} RadioMode;

/// \brief Radio frequency bands.
typedef enum {
  RADIO_BAND_VHF,         ///< Very High Frequency band.
  RADIO_BAND_UHF,         ///< Ultra High Frequency band.
  RADIO_BAND_LPD,         ///< Low Power Device band.
  RADIO_BAND_UNSUPPORTED  ///< Unsupported frequency band.
} RadioBand;

/// \brief Select the antenna based on satellite radio mode and frequency band.
/// \param Mode The radio mode from #RadioMode.
/// \param Band The radio frequency band from #RadioBand.
/// \return 0 if succeeded, -1 if failed.
int BoardAntennaSelect(RadioMode Mode, RadioBand Band);

/// @}

/// @defgroup Board_debug Debug output
/// \brief Debug serial interface for logging and diagnostics.
/// @{

/// \brief Initialise the debug output interface.
/// \details Configures the debug serial interface for logging and diagnostics.
/// \return Void pointer to debug handle, or NULL if initialisation fails.
void *BoardDebugInit(void);
/// \brief Deinitialise the debug output interface.
/// \details Closes the debug port and releases associated resources.
void BoardDebugDeinit(void);
/// \brief Write data to the debug output interface.
/// \param Tx[in] Pointer to the data buffer to be transmitted.
/// \param Length Number of bytes to transmit.
/// \return 0 if all bytes are sent successfully, -1 if transmission fails.
int BoardDebugWrite(const uint8_t *Tx, size_t Length);
/// \brief Read data from the debug output interface.
/// \param Rx[out] Pointer to the buffer to store received data.
/// \param Length Number of bytes to read.
/// \return Number of bytes read, or -1 if read fails.
int BoardDebugRead(uint8_t *Rx, size_t Length);

/// @}

/// @defgroup Board_sleep_hook Sleep hooks
/// \brief System power management hooks for sleep and wake events.
/// @{

/// \brief Hook invoked before the system enters sleep.
/// \details Called by the system before entering a low-power sleep state.
void BoardSleepEnter(void);
/// \brief Hook invoked after the system wakes up from sleep.
/// \details Called by the system immediately after waking from low-power sleep
/// mode.
/// \see BoardSleepEnter()
void BoardSleepExit(void);

/// @}

/// @}

#endif  // MYRIOTA_BSP_H
