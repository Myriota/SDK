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

/// SDK version
void SDKVersionGet(uint32_t *pMajor, uint32_t *pMinor, uint32_t *pPatch);

/// User defined application initialisation
// Will be called at terminal reset/restart.
void AppInit(void);

/// User definable jobs
// The return value is the time at which the job should next run.
typedef time_t (*ScheduledJob)(void);
// Reset the schedule for an existing job or add a new job.
// Return 0 on success, -1 if maximum number of jobs reached
int ScheduleJob(ScheduledJob Job, time_t Time);

/// Time/scheduling functions
time_t ASAP(void);
time_t Never(void);
time_t SecondsFromNow(unsigned Secs);
time_t MinutesFromNow(unsigned Mins);
time_t HoursFromNow(unsigned Hours);
time_t DaysFromNow(unsigned Days);
time_t OnGPIOWakeup(void);

/// Return a time after "after" and before "before" immediately prior to a
// satellite transmit opportunity. Can be used to reduce latency between sensor
// reading and satellite transmission. If no opportunity is found in the
// interval [after, before], then "before" is returned.
time_t BeforeSatelliteTransmit(time_t After, time_t Before);

/// Maximum size in bytes of individual transmit message.
#define MAX_MESSAGE_SIZE 20

/// Schedule a message of bytes of given size for transmission.
// The maximum message size is given by MAX_MESSAGE_SIZE.
// Returns message load. Return values greater than one indicate that messages
// are being scheduled too quickly. NAN is returned if an error occurs, for
// example, if the MessageSize exceeds MAX_MESSAGE_SIZE
float ScheduleMessage(const uint8_t *Message, size_t MessageSize);

/// Delays
// Delay for a number of milliseconds
void Delay(uint32_t mSec);
// Put the system in lower power mode for Sec seconds.
// The current job won't be interrupted by other jobs while sleeping.
void Sleep(uint32_t Sec);

/// GNSS
// Get a time and location fix from GNSS.
// Updates real-time clock and latitude and longitude values.
// Returns 0 if GNSS fix was correctly obtained and -1 otherwise.
int GNSSFix(void);
// Returns true if the system has obtained a valid location and time fix.
bool HasValidGNSSFix(void);
// Returns last recorded latitude and longitude in fixed point format and
// timestamp of last fix. Values are in degrees multiplied by 1e7.
void LocationGet(int32_t *Latitude, int32_t *Longitude, time_t *TimeStamp);

/// Error logging
// Log errors. Error codes are user defined (0 - 127)
// The size can be 0 to log only the error code
// Returns 0 if logging succeeds, -1 if logging fails
int LogAdd(uint8_t ErrorCode, const void *Buffer, uint8_t BufferSize);

/// Time
// Get current epoch time
time_t TimeGet(void);

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_USER_API_H
