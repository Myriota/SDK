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

// User programming interface for the Myriota terminal

#ifndef MYRIOTA_USER_API_H
#define MYRIOTA_USER_API_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "myriota_hardware_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup System_api System API
/// Non hardware interface related API
/// @{

/// @defgroup SDK_version SDK version
/// @{

void SDKVersionGet(uint32_t *Major, uint32_t *Minor, uint32_t *Patch);

/// @}

/// @defgroup App_init Application initialisation
/// User defined application initialisation logic
/// @{

/// Must be implemented by the application.
/// Will be called at device startup before any application jobs.
void AppInit(void);

/// @}

/// @defgroup Time_schedule Time and job scheduling
/// @{

/// The return value is the time at which the job should next run.
typedef time_t (*ScheduledJob)(void);
/// Reset the schedule for an existing job or add a new job.
/// Return 0 if succeeded and -1 if maximum number of jobs reached.
int ScheduleJob(ScheduledJob Job, time_t Time);
time_t ASAP(void);
time_t Never(void);
time_t SecondsFromNow(unsigned Secs);
time_t MinutesFromNow(unsigned Mins);
time_t HoursFromNow(unsigned Hours);
time_t DaysFromNow(unsigned Days);
/// GPIO wakeup event
time_t OnGPIOWakeup(void);
/// Pulse counter overflow event
time_t OnPulseCounterEvent(void);
/// Return a time after \p After and before \p Before immediately prior to a
/// satellite transmit opportunity. Can be used to reduce latency between sensor
/// reading and satellite transmission. If no opportunity is found between \p
/// After and \p Before, then \p Before is returned.
time_t BeforeSatelliteTransmit(time_t After, time_t Before);

/// @}

/// @defgroup User_message User message
/// @{

/// Maximum size in bytes of individual transmit message.
#define MAX_MESSAGE_SIZE 20
/// Schedule a message of bytes of given size for transmission.
/// The maximum message size is given by #MAX_MESSAGE_SIZE.
/// Returns message load. Return values greater than one indicate that messages
/// are being scheduled too quickly. NAN is returned if an error occurs, for
/// example, if the \p MessageSize exceeds #MAX_MESSAGE_SIZE.
float ScheduleMessage(const uint8_t *Message, size_t MessageSize);

/// @}

/// @defgroup Delay Delays
/// @{

/// Delay for a number of milliseconds
void Delay(uint32_t mSec);
/// Put the system in lower power mode for Sec seconds.
/// The current job won't be interrupted by other jobs while sleeping.
void Sleep(uint32_t Sec);

/// @}

/// @defgroup Time_location Time and location
/// @{

/// Get a time and location fix from GNSS.
/// Updates real-time clock and latitude and longitude values.
/// Returns 0 if GNSS fix was correctly obtained and -1 otherwise.
int GNSSFix(void);
/// Returns true if the system has obtained a valid time and location fix.
bool HasValidGNSSFix(void);
/// Returns last recorded latitude and longitude in fixed point format and
/// timestamp of last fix. Values are in degrees multiplied by 1e7.
void LocationGet(int32_t *Latitude, int32_t *Longitude, time_t *TimeStamp);
/// Get current epoch time
time_t TimeGet(void);

/// @}

/// @defgroup Temp_sensor Temperature sensor
/// @{

/// Get on board temperature in degrees Celsius
/// Returns 0 if succeeded and -1 if failed
int TemperatureGet(float *Temperature);

/// @}

/// @defgroup LED_control LED control
/// @{

void LedTurnOn(void);
void LedTurnOff(void);
void LedToggle(void);

/// @}

/// @defgroup Battery Battery voltage
/// @{

/// Get the battery voltage in millivolt.
/// Returns 0 if succeeded and -1 if failed.
/// If battery measurement is not supported then both voltage and the return
/// value are 0.
int BatteryGetVoltage(uint32_t *mV);

/// @}

/// @defgroup System_tick System tick
/// @{

/// Get the current system tick (1000 ticks per second).
uint32_t TickGet(void);

/// @}

/// @defgroup Error_logging Error logging
/// @{

/// Log errors. Error codes are user defined (0 - 127).
/// The size can be 0 to log only the error code
/// Returns 0 if logging succeeds and -1 if logging fails.
int LogAdd(uint8_t ErrorCode, const void *Buffer, uint8_t BufferSize);

/// @}

/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_USER_API_H
