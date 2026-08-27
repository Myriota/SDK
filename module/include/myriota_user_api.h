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
/// \brief Non hardware interface related API.
/// @{

/// @defgroup SDK_version SDK version
/// \brief SDK version information for runtime compatibility checks.
/// @{

/// \brief Get the SDK version numbers.
/// \details Retrieves the semantic version of the SDK as three separate major,
/// minor, and patch version numbers. This allows applications to determine
/// which SDK features and APIs are available at runtime.
/// \param[out] Major Pointer to store the major version number
/// (e.g., 1 for v1.2.3).
/// \param[out] Minor Pointer to store the minor version number
/// (e.g., 2 for v1.2.3).
/// \param[out] Patch Pointer to store the patch version number
/// (e.g., 3 for v1.2.3).
void SDKVersionGet(uint32_t *Major, uint32_t *Minor, uint32_t *Patch);

/// @}

/// @defgroup ModuleID_get Get module ID
/// \brief Get the module ID string and part number.
/// @{

/// \brief Get the module ID string.
/// \return The string of module ID in the format of "00xxxxxxxx Mx-2x" where
/// 00xxxxxxxx is the module ID in hexadecimal string and Mx-2x is the module's
/// part number. Returns null if module ID is not available.
char *ModuleIDGet(void);

/// \brief Get the registration code string.
/// \return The string of registration code. Returns null if registration code
/// is not available.
char *RegistrationCodeGet(void);

/// @}

/// @defgroup App_init Application initialisation
/// \brief User defined application initialisation logic.
/// @{

/// \brief Call user-defined application initialisation function.
/// \details Must be implemented by the application. Will be called at device
/// startup before any application jobs.
void AppInit(void);

/// @}

/// @defgroup Time_schedule Time and job scheduling
/// \brief Job scheduling callbacks and time utility functions for periodic
/// execution.
/// @{

/// \brief Custom job scheduling function type.
/// \details Defines a function pointer type for scheduled jobs that return a
/// time_t value indicating the next scheduled execution time.
typedef time_t (*ScheduledJob)(void);
/// \brief Schedule a job to run at a specific time (or reset the schedule for
/// an existing job).
/// \param Job The job function to be scheduled.
/// \param Time The time at which the job should run.
/// \return 0 if the job was successfully scheduled, -1 if the maximum number
/// of jobs is reached or an error occurs.
int ScheduleJob(ScheduledJob Job, time_t Time);
/// \brief Returns the next scheduling time for a job that should run as soon
/// as possible.
/// \return A time_t value representing the next scheduled time for the job if
/// HasValidGNSSFix() is true; otherwise, returns 0.
time_t ASAP(void);
/// \brief Returns a time value used to indicate that a job should not be
/// scheduled.
/// \return A time_t value representing a time far in the future, effectively
/// representing "never" for scheduling purposes.
time_t Never(void);
/// \brief Returns a value that is seconds from the current time.
/// \param Secs The number of seconds from the current time to calculate the
/// future time.
/// \return A time_t value representing the time that is Secs seconds from now.
/// If Secs is 0, returns the current time.
time_t SecondsFromNow(unsigned Secs);
/// \brief Returns a value that is minutes from the current time.
/// \param Mins The number of minutes from the current time to calculate the
/// future time.
/// \return A time_t value representing the time that is Mins minutes from now.
/// If Mins is 0, returns the current time.
time_t MinutesFromNow(unsigned Mins);
/// \brief Returns a value that is hours from the current time.
/// \param Hours The number of hours from the current time to calculate the
/// future time.
/// \return A time_t value representing the time that is Hours hours from now.
/// If Hours is 0, returns the current time.
time_t HoursFromNow(unsigned Hours);
/// \brief Returns a value that is days from the current time.
/// \param Days The number of days from the current time to calculate the
/// future time.
/// \return A time_t value representing the time that is Days days from now.
/// If Days is 0, returns the current time.
time_t DaysFromNow(unsigned Days);
/// \brief Returns a value used to represent a GPIO wakeup events.
/// \return A time_t value representing the time for a GPIO wakeup event.
time_t OnGPIOWakeup(void);
/// \brief Returns a value used to represent a pulse counter events.
/// \return A time_t value representing the time for a pulse counter event.
time_t OnPulseCounterEvent(void);
/// \brief Returns a value used to represent a Leuart activity event.
/// \return A time_t value representing the time for a Leuart activity event.
time_t OnLeuartReceive(void);
/// \brief Return a time after \p After and before \p Before immediately prior
/// to a satellite transmit opportunity.
/// \details Can be used to reduce latency between sensor reading and satellite
/// transmission. If no opportunity is found between \p After and \p Before,
/// then \p Before is returned.
/// \param After The start of the time interval to check for satellite passes.
/// \param Before The end of the time interval to check for satellite passes.
/// \return The time of the next satellite pass within the interval, or
/// \p Before if no pass is found.
time_t BeforeSatelliteTransmit(time_t After, time_t Before);
/// @}

/// @defgroup User_message User message
/// \brief Transmit message scheduling and queue management.
/// @{

/// \brief Schedule a message of the given size for transmission.
/// \details Calling ScheduleMessage when MessageSlotsFree returns 0 replaces
/// an existing message in the queue, which may result in dropped messages.
/// Message IDs are sequential unsigned 16-bit integers and start at zero. IDs
/// are guaranteed to be unique across messages that are in the queue at any
/// given time (IDs wrap after 65535).
/// \param Message[in] Pointer to the message data.
/// \param MessageSize Size of the message in bytes.
/// \return Message ID (>=0) if successfully scheduled, or <0 on failure.
/// \see MessageSlotsFree(), MessageBytesFree(), MessageSlotsMax()
int ScheduleMessage(const uint8_t *Message, size_t MessageSize);
/// \brief Returns the number of available slots in the internal queue.
/// \return The number of available slots in the internal queue, that is,
/// the number of messages that can be scheduled with ScheduleMessage.
int MessageSlotsFree(void);
/// \brief Returns the number of bytes remaining in the internal queue.
/// \return The number of bytes remaining in the internal queue, that is,
/// the number of bytes that can be scheduled with ScheduleMessage.
size_t MessageBytesFree(void);
/// \brief Returns the maximum number of message slots available in the internal
/// message queue.
/// \return The maximum number of message slots available in the internal
/// message queue. This value represents the total capacity of the queue,
/// independent of how many slots are currently free.
int MessageSlotsMax(void);
/// \brief Save all messages in the message queue to module's persistent
/// storage.
/// \details Saved messages will be transmitted after reset.
/// \see MessageQueueClear()
void SaveMessages(void);
/// \brief Clear all messages in the message queue.
/// \details Clears the in-memory message queue, discarding all pending
/// messages that have not yet been transmitted.
/// \see SaveMessages(), ScheduleMessage()
void MessageQueueClear(void);

/// \brief Message transmission status
typedef enum {
  TRANSMIT_PENDING,   ///< Message not yet transmitted.
  TRANSMIT_ONGOING,   ///< Message is currently being transmitted.
  TRANSMIT_COMPLETE,  ///< Message fully transmitted.
  TRANSMIT_EXPIRED,   ///< Message expired before completion.
} MessageTransmitStatus_t;

/// \brief Status information for a message in the uplink queue.
typedef struct {
  uint16_t id;                     ///< ID returned by ScheduleMessage API.
  MessageTransmitStatus_t status;  ///< Status of the message.
} MessageStatus_t;
/// \brief Returns the status of messages in the message queue.
/// \details Fills up to status_count entries in the status array.
/// \param status[out] Pointer to an array of MessageStatus_t structures to be
/// filled with message status information.
/// \param status_count The maximum number of entries that can be written to
/// the status array.
/// \return The number of entries written, or -1 if invalid input.
int MessageQueueStatus(MessageStatus_t *status, const int status_count);
/// \brief Deletes a message from the message queue based on its message ID.
/// \param message_id The ID of the message to delete.
/// \return 0 if the message was successfully deleted, or -1 if the ID was not
/// found.
int MessageQueueDelete(const uint16_t message_id);
/// \brief Maximum size in bytes of individual transmit message.
/// \deprecated This constant is DEPRECATED and should not be used.
#define MAX_MESSAGE_SIZE 20

/// @}

/// @defgroup Device_Control Device control
/// \brief Downlink message reception and device control.
/// @{

/// \brief Returns a value used to represent a received user message event.
/// \return A time_t value representing the time for a received user message
/// event.
time_t OnReceiveMessage(void);

/// \brief Retrieve the received user message.
/// \details The length of received message is indicated by \p size.
/// \param size[out] pointer to an integer to store the size of the received
/// message.
/// \return a pointer to the message if size is greater than 0 or NULL
/// otherwise.
uint8_t *ReceiveMessage(int *size);

/// @}

/// @defgroup Device_Delays Delay Delays
/// \brief Timing functions for delays and sleep modes.
/// @{

/// \brief Delays execution for a specified number of milliseconds.
/// \details This is a blocking operation and should be used sparingly in the
/// scheduler context.
/// \param mSec Duration to delay in milliseconds.
/// \note Use Delay() only for sub-second timing or hardware setup delays.
/// \see Sleep()
void Delay(uint32_t mSec);
/// \brief Delays execution for a specified number of microseconds.
/// \details This is a blocking operation and should be used sparingly in the
/// scheduler context.
/// \param uSec Duration to delay in microseconds.
/// \note Use MicroSecondDelay() only for sub-second timing or hardware setup
/// delays.
/// \see Sleep()
void MicroSecondDelay(uint32_t uSec);
/// \brief Put the system in lower power mode for a specified number of seconds.
/// \details The calling job won't be interrupted by other jobs except for
/// events while sleeping.
/// \param Sec Duration to sleep in seconds.
void Sleep(uint32_t Sec);

/// @}

/// @defgroup Time_location Time and location
/// \brief GNSS positioning and time synchronisation.
/// @{

/// \brief Perform a time and location fix from GNSS.
/// \details Updates real-time clock and latitude and longitude values.
/// \return 0 if GNSS fix was correctly obtained and -1 otherwise.
int GNSSFix(void);
/// \brief Check if the system has a valid GNSS fix.
/// \return true if a valid GNSS fix is available, false otherwise.
bool HasValidGNSSFix(void);
/// \brief Get the last recorded location and timestamp.
/// \param Latitude[out] Pointer to store the latitude in degrees multiplied by
/// 1e7.
/// \param Longitude[out] Pointer to store the longitude in degrees multiplied
/// by 1e7.
/// \param TimeStamp[out] Pointer to store the timestamp of the last fix.
void LocationGet(int32_t *Latitude, int32_t *Longitude, time_t *TimeStamp);
/// \brief Set the location.
/// \details Location set may be overridden by GNSS fix if GNSS location fix is
/// enabled.
/// \param Latitude Latitude in degrees multiplied by 1e7.
/// \param Longitude Longitude in degrees multiplied by 1e7.
void LocationSet(int32_t Latitude, int32_t Longitude);
/// \brief Get the current epoch time.
/// \return The current epoch time.
time_t TimeGet(void);
/// \brief Set the current epoch time.
/// \param Time The epoch time to set.
void TimeSet(time_t Time);

/// @}

/// @defgroup Temp_sensor Temperature sensor
/// \brief Internal module temperature measurement.
/// @{

/// \brief Get the temperature inside the module in degrees Celsius.
/// \param Temperature[out] Pointer to store the temperature value.
/// \return 0 if succeeded and -1 if failed.
int TemperatureGet(float *Temperature);

/// @}

/// @defgroup LED_control LED control
/// \brief Status LED control for system indication.
/// @{

/// \brief Turn on the status LED.
/// \details Powers on the status LED.
void LedTurnOn(void);
/// \brief Turn off the status LED.
/// \details Powers off the status LED.
void LedTurnOff(void);
/// \brief Toggle the status LED state.
/// \details Switches the LED to the opposite state: if on, turns it off; if
/// off, turns it on. Useful for visual feedback without tracking the LED state
/// manually.
void LedToggle(void);

/// @}

/// @defgroup Battery Battery voltage
/// \brief Battery voltage measurement and monitoring.
/// @{

/// \brief Get the battery voltage in millivolt.
/// \param mV[out] Pointer to store the battery voltage in millivolt.
/// \return 0 if succeeded and -1 if failed.
/// \note If battery measurement is not supported then both voltage and the
/// return value are 0.
int BatteryGetVoltage(uint32_t *mV);

/// @}

/// @defgroup System_tick System tick
/// \brief High-resolution system timer access.
/// @{

/// \brief Get the current system tick.
/// \details The system tick increments at a rate of 1000 ticks per second (1
/// tick = 1 millisecond). It can be used for measuring elapsed time,
/// implementing timeouts, or scheduling tasks with millisecond precision.
/// \return The current system tick in milliseconds.
uint32_t TickGet(void);

/// @}

/// @defgroup Error_logging Error logging
/// \brief Application error and diagnostic logging.
/// @{

/// \brief Log errors.
/// \details The error codes are user-defined and should be in the range of 0
/// to 127. If BufferSize is set to 0, only the error code will be logged
/// without any additional data.
/// \param ErrorCode The error code to log.
/// \param Buffer[out] Pointer to the buffer containing additional log data.
/// \param BufferSize Size of the buffer in bytes.
/// \return 0 if logging succeeds and -1 if logging fails.
int LogAdd(uint8_t ErrorCode, const void *Buffer, uint8_t BufferSize);

/// @}

/// @defgroup Suspend_mode Suspend mode
/// \brief System suspend and wake management.
/// @{

/// \brief Enable or disable suspend mode.
/// \details This has no effect if the system is already in the same state.
/// \param Enable true to enable suspend mode, false to disable.
void SuspendModeEnable(bool Enable);

/// Returns true if suspend mode is enabled and false if disabled.
bool SuspendModeIsEnabled(void);

/// @}

/// @defgroup System_update System update
/// \brief Update or get system information.
/// \details Interface for user to update or retrieve selected system
/// information. i.e. to update network information. It is recommended that the
/// entire process done in a single task window.
/// @{

/// \brief System update or get operation IDs.
enum SystemUpdateID {
  ///< Start of system update operation IDs.
  SYSTEM_UPDATE_ID_WRITE_START = 0,
  ///< Update network information.
  SYSTEM_UPDATE_ID_NETWORK = SYSTEM_UPDATE_ID_WRITE_START,
  ///< Start of system get operation IDs.
  SYSTEM_GET_ID_START = (1 << 4),
  ///< Get network information.
  SYSTEM_GET_ID_NETWORK = SYSTEM_GET_ID_START,
  ///< Get diagnostics information.
  SYSTEM_GET_ID_DIAGNOSTICS
};

/// \brief Initiate a system update or get request.
/// \details Pass 0 to \p Size for get operation. Update or get will be
/// cancelled automatically when the \p Timeout expires.
/// \param ID The ID of the system update or get operation \ref SystemUpdateID.
/// \param Size The size of the data to be transferred.
/// \param Timeout The maximum time in seconds for the entire operation.
/// \return  For update operation: returns 0 if the update starts successfully
/// and -1 if \p ID is unknown, \p Size is too big or there is an ongoing
/// operation. For get operation: returns number of bytes to transfer if it
/// starts successfully and -1 if \p ID is unknown, or there is an ongoing
/// operation.
int SystemUpdateStart(uint8_t ID, uint32_t Size, uint32_t Timeout);

/// \brief System update or get data transfer.
/// \details For the best transfer speed, it is recommended that \p Offset and
/// \p BufSize are set to multiple of 2048 bytes where possible.
/// \param Offset The offset in the data buffer.
/// \param Buf[out] The data buffer.
/// \param BufSize The size of the data buffer.
/// \return For update operation: returns 0 if the transfer succeeds and -1 if
/// it fails. For get operation: returns the number of bytes written to \p Buf
/// if the transfer succeeds and -1 if it fails.
int SystemUpdateXfer(uint32_t Offset, uint8_t *Buf, size_t BufSize);

/// \brief Indicates the finish of the content transfer.
/// \details For update operation, actual update will be performed if the
/// transferred data are validated.
/// \return 0 on success and -1 on failure.
int SystemUpdateFinish(void);

/// @}

/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_USER_API_H
