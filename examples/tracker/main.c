// Copyright (c) 2016-2020, Myriota Pty Ltd, All Rights Reserved
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

// A simple tracker application. Sends messages to satellite
// containing message sequence number, the current device
// location, and the time.

#include "myriota_user_api.h"

// Format of the messages to be transmitted. Values are little endian
typedef struct {
  uint16_t sequence_number;
  int32_t latitude;   // scaled by 1e7, e.g. -891234567 (south 89.1234567)
  int32_t longitude;  // scaled by 1e7, e.g. 1791234567 (east 179.1234567)
  uint32_t time;      // epoch timestamp of last fix
} __attribute__((packed)) tracker_message;

static time_t Tracker(void) {
  static unsigned int sequence_number = 0;

  if (GNSSFix()) printf("Failed to get GNSS Fix, using last known fix\n");

  // The timestamp and the coordinate are incorrect without valid GNSS fix
  if (HasValidGNSSFix()) {
    int32_t lat, lon;
    time_t timestamp;
    LocationGet(&lat, &lon, &timestamp);

    const tracker_message message = {sequence_number, lat, lon, timestamp};

    ScheduleMessage((void *)&message, sizeof(message));

    printf("Scheduled message: %u %f %f %u\n", sequence_number, lat * 1e-7,
           lon * 1e-7, (unsigned int)timestamp);

    sequence_number++;
  }

  // At most 3 messages a day, at least 1. Minimise latency.
  // When there is no valid GNSS fix, the job will be
  // rescheduled straightaway as soon as a valid GNSS has been acquired
  return BeforeSatelliteTransmit(HoursFromNow(8), HoursFromNow(24));
}

void AppInit() { ScheduleJob(Tracker, ASAP()); }
