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

// A simple tracker application. Sends messages to satellite
// containing message sequence number, the current device
// location, and the time.

#include <stdlib.h>
#include <string.h>
#include "myriota_user_api.h"

#ifndef LOCATIONS_PER_MESSAGE
#define LOCATIONS_PER_MESSAGE 4
#endif
#ifndef TRACKER_INTERVAL_HRS
#define TRACKER_INTERVAL_HRS 2
#endif

// Location entry with latitude, longitude, and timestamp. Values are little
// endian.
typedef struct {
  int32_t latitude;   // scaled by 1e7, e.g. -891234567 (south 89.1234567)
  int32_t longitude;  // scaled by 1e7, e.g. 1791234567 (east 179.1234567)
  uint32_t time;      // epoch timestamp of location record
} __attribute__((packed)) location_t;

// Format of the messages to be transmitted.
typedef struct {
  uint16_t sequence_number;  // Sequence number of the message
  uint8_t location_count;    // Number of location entries in the below array
  location_t locations[LOCATIONS_PER_MESSAGE];  // Array of location entries

} __attribute__((packed)) tracker_message;

static tracker_message msg = {0, 0, {{0}}};

static void SendMessage(const tracker_message *msg) {
  if (ScheduleMessage((void *)msg, sizeof(tracker_message)) == -1) {
    printf("Failed to send message: %u %u", msg->sequence_number,
           msg->location_count);
    return;
  }

  printf("Scheduled message: %u %u", msg->sequence_number, msg->location_count);
  for (int i = 0; i < msg->location_count; i++)
    printf(" %f %f %u", msg->locations[i].latitude / 1e7,
           msg->locations[i].longitude / 1e7,
           (unsigned int)msg->locations[i].time);
  printf("\n");
}

static time_t TrackerJob(void) {
  if (GNSSFix()) printf("Failed to get GNSS Fix, using last known fix\n");

  // The timestamp and the coordinate are incorrect without valid GNSS fix
  if (!HasValidGNSSFix()) return HoursFromNow(TRACKER_INTERVAL_HRS);

  int32_t lat;
  int32_t lon;
  time_t timestamp;
  LocationGet(&lat, &lon, &timestamp);

  location_t *location = &msg.locations[msg.location_count];
  location->latitude = lat;
  location->longitude = lon;
  location->time = (uint32_t)timestamp;
  msg.location_count++;

  if (msg.location_count < LOCATIONS_PER_MESSAGE)
    return HoursFromNow(TRACKER_INTERVAL_HRS);  // Still collecting locations

  // Message ready for transmission
  SendMessage(&msg);

  msg.sequence_number++;
  msg.location_count = 0;

  return HoursFromNow(TRACKER_INTERVAL_HRS);
}

void AppInit() { ScheduleJob(TrackerJob, ASAP()); }
