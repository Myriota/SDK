// Copyright (c) 2016-2025, Myriota Pty Ltd, All Rights Reserved
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

// User programming interface for the Myriota Module

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

/// @defgroup ModuleID_get Get module ID string and part number
/// @{

/// Returns the string of module ID in the format of "00xxxxxxxx Mx-2x" where
/// 00xxxxxxxx is the module ID in hexadecimal string and Mx-2x is the module's
/// part number. Returns null if module ID is not available.
char *ModuleIDGet(void);

/// Returns the string of registration code. Returns null if registration code
/// is not available.
char *RegistrationCodeGet(void);

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
/// Leuart activity event
time_t OnLeuartReceive(void);
/// Return a time after \p After and before \p Before immediately prior to a
/// satellite transmit opportunity. Can be used to reduce latency between sensor
/// reading and satellite transmission. If no opportunity is found between \p
/// After and \p Before, then \p Before is returned.
time_t BeforeSatelliteTransmit(time_t After, time_t Before);
/// @}

/// @defgroup User_message User message
/// @{

/// Schedule a message of the given size for transmission.
/// Returns a message ID (>=0) when successfully scheduled, or <0 on failure.
/// Calling ScheduleMessage when MessageSlotsFree returns 0 replaces an existing
/// message in the queue, which may result in dropped messages. See also
/// MessageSlotsFree. Message IDs are sequential unsigned 16-bit integers and
/// start at zero. IDs are guaranteed to be unique across messages that are in
/// the queue at any given time (IDs wrap after 65535).
int ScheduleMessage(const uint8_t *Message, size_t MessageSize);
/// Returns the number of available slots in the internal queue,
/// that is, the number of messages that can be scheduled with ScheduleMessage.
int MessageSlotsFree(void);
/// Returns number of bytes remaining in the internal queue, that is,
/// the number of bytes that can be scheduled with ScheduleMessage.
size_t MessageBytesFree(void);
/// Returns the maximum number of message slots available in the internal
/// message queue. This value represents the total capacity of the queue,
/// independent of how many slots are currently free.
int MessageSlotsMax(void);
/// Save all messages in the message queue to module's persistent storage.
/// Saved messages will be transmitted after reset.
void SaveMessages(void);
/// Clear all messages in the message queue.
void MessageQueueClear(void);

/// Message transmission status
typedef enum {
  TRANSMIT_PENDING,   ///< Message not yet transmitted
  TRANSMIT_ONGOING,   ///< Message is currently being transmitted
  TRANSMIT_COMPLETE,  ///< Message fully transmitted
  TRANSMIT_EXPIRED,   ///< Message expired before completion
} MessageTransmitStatus_t;

/// Status information for a message in the uplink queue
typedef struct {
  uint16_t id;                     ///< id returned by ScheduleMessage API
  MessageTransmitStatus_t status;  ///< status of the message
} MessageStatus_t;
/// Returns the status of messages in the message queue.
/// Fills up to status_count entries in the status array.
/// Returns the number of entries written, or -1 if invalid input.
int MessageQueueStatus(MessageStatus_t *status, const int status_count);
/// Deletes a message from the message queue based on its message ID.
/// Returns 0 if the message was successfully deleted, or -1 if the ID was not
/// found.
int MessageQueueDelete(const uint16_t message_id);
/// Maximum size in bytes of individual transmit message.
/// This constant is DEPRECATED and should not be used.
#define MAX_MESSAGE_SIZE 20

/// @}

/// @defgroup Device Control
/// @{

// Received user message event
time_t OnReceiveMessage(void);

/// Retrieve the received user message. The length of received message is
/// indicated by \p size. Returns a pointer to the message if size is
/// greater than 0 or NULL otherwise.
uint8_t *ReceiveMessage(int *size);

/// @}

/// @defgroup Delay Delays
/// @{

/// Delay for a number of milliseconds
void Delay(uint32_t mSec);
/// Delay for a number of microseconds
void MicroSecondDelay(uint32_t uSec);
/// Put the system in lower power mode for \p Sec seconds.
/// The calling job won't be interrupted by other jobs except for events while
/// sleeping.
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
/// Set the location, Lat and Lon are in degrees multiplied by 1e7.
/// Location set may be overridden by GNSS fix if GNSS location fix is enabled.
void LocationSet(int32_t Latitude, int32_t Longitude);
/// Get current epoch time
time_t TimeGet(void);
/// Set current epoch time
void TimeSet(time_t Time);

/// @}

/// @defgroup Temp_sensor Temperature sensor
/// @{

/// Get the temperature inside the module in degrees Celsius
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

/// @defgroup Suspend_mode Suspend mode
/// @{

/// Change suspend mode state. Set true to enable and false to disable. Has no
/// effect if the system is already in the same state.
void SuspendModeEnable(bool Enable);

/// Returns true if suspend mode is enabled and false if disabled.
bool SuspendModeIsEnabled(void);

/// @}

/// @defgroup Update or get system information.
/// Interface for user to update or retrieve selected system information. i.e.
/// To update network information. It is recommended that the entire process
/// done in a single task window.
/// @{

/// System update/get IDs
enum SystemUpdateID {
  SYSTEM_UPDATE_ID_WRITE_START = 0,
  SYSTEM_UPDATE_ID_NETWORK = SYSTEM_UPDATE_ID_WRITE_START,
  SYSTEM_GET_ID_START = (1 << 4),
  SYSTEM_GET_ID_NETWORK = SYSTEM_GET_ID_START,
  SYSTEM_GET_ID_DIAGNOSTICS
};

/// Initiate a system update or get request, with id, size, and maximum
/// time in seconds for the entire operation. Pass 0 to \p Size for get
/// operation. Update or get will be cancelled automatically when the \p Timeout
/// expires.
/// For update operation: returns 0 if the update starts successfully
/// and -1 if \p ID is unknown, \p Size is too big or there is an ongoing
/// operation.
/// For get operation: returns number of bytes to transfer if it starts
/// successfully and -1 if \p ID is unknown, or there is an ongoing operation.
int SystemUpdateStart(uint8_t ID, uint32_t Size, uint32_t Timeout);

/// System update or get data transfer.
/// For update operation: returns 0 if the transfer succeeds and -1 if it fails.
/// For the best transfer speed, it is recommended that \p Offset and \p
/// BufSize are set to multiple of 2048 bytes where possible.
/// For get operation: returns the number of bytes written to \p Buf if the
/// transfer succeeds and -1 if it fails.
int SystemUpdateXfer(uint32_t Offset, uint8_t *Buf, size_t BufSize);

/// Indicates the finish of the content transfer.
/// For update operation, actual update will be
/// performed if the transferred data are validated.
/// Returns 0 on succeess and -1 on failure.
int SystemUpdateFinish(void);

/// @}

/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_USER_API_H
