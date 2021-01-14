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

// An example showing how to use the MaxThroughput API to scale up or down the
// number of messages scheduled in accordance with the capacity of the satellite
// network. Caps the maximum number of messages transmitted per day at 24.

#include "myriota_user_api.h"

// Format of the messages to be transmitted. Values are little endian
typedef struct {
  uint16_t sequence_number;
  uint32_t time;
} __attribute__((packed)) throughput_message;

static time_t Throughput(void) {
  static unsigned int sequence_number = 0;

  const throughput_message message = {sequence_number, TimeGet()};

  ScheduleMessage((void *)&message, sizeof(message));

  printf("Scheduled message: %u %u\n", message.sequence_number,
         (unsigned int)message.time);

  sequence_number++;

  // return time for maximum throughput capped at 24 messages per day
  return MaxThroughput(24);
}

void AppInit() { ScheduleJob(Throughput, ASAP()); }
