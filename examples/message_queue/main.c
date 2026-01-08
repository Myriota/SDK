// Copyright (c) 2025, Myriota Pty Ltd, All Rights Reserved
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

// Example application demonstrating the message queue APIs.
// Schedules alternating 20-byte and 50-byte messages, embeds a sequence number
// in each payload, and prints queue status each run.

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "myriota_user_api.h"

#ifndef MESSAGES_PER_DAY
#define MESSAGES_PER_DAY 3
#endif

typedef struct {
  uint16_t sequence_number;
  uint32_t time;
} __attribute__((packed)) simple_message;

// Convert status enum to string
static const char *StatusString(const MessageTransmitStatus_t s) {
  switch (s) {
    case TRANSMIT_PENDING:
      return "PENDING";
    case TRANSMIT_ONGOING:
      return "ONGOING";
    case TRANSMIT_COMPLETE:
      return "COMPLETE";
    case TRANSMIT_EXPIRED:
      return "EXPIRED";
  }
  return "UNKNOWN";
}

// Print a compact, grouped queue summary
static void PrintQueueInfo(void) {
  const int max = MessageSlotsMax();
  MessageStatus_t status[max];
  const int count = MessageQueueStatus(status, max);

  printf("\n--- Queue Snapshot ---\n");
  printf("MessageSlotsFree: %d / %d\n", MessageSlotsFree(), max);
  printf("MessageBytesFree: %d\n\n", (int)MessageBytesFree());

  if (count <= 0) {
    printf("Queue is empty.\n\n");
    return;
  }

  printf("TransmitStatus   MessageIDs\n");
  for (MessageTransmitStatus_t s = TRANSMIT_PENDING; s <= TRANSMIT_EXPIRED;
       s++) {
    printf("%-16s ", StatusString(s));
    for (int i = 0; i < count; i++)
      if (status[i].status == s) printf("%u ", status[i].id);
    printf("\n");
  }
  printf("\n");
}

static time_t MessageQueueExampleJob(void) {
  PrintQueueInfo();

  // Alternate between 20B and 50B payloads, embed a sequence number
  static uint16_t sequence_number = 0;
  const int msg_len = (sequence_number % 2 == 0) ? 20 : 50;
  const time_t next_job_run =
      HoursFromNow(MessageSlotsMax() / MESSAGES_PER_DAY);

  if (MessageSlotsFree() == 0) {
    printf("No free message slots available\n");
    return next_job_run;
  }

  if (MessageBytesFree() < msg_len) {
    printf("Not enough free bytes for a %d-byte message\n", msg_len);
    return next_job_run;
  }

  uint8_t msg[msg_len];
  memset(msg, 0, msg_len);
  const simple_message header = {.sequence_number = sequence_number,
                                 .time = (uint32_t)time(NULL)};
  memcpy(msg, &header, sizeof(header));

  const int id = ScheduleMessage(msg, msg_len);
  if (id < 0) {
    printf("Failed to schedule message of %d bytes\n", msg_len);
    return next_job_run;
  }

  printf("Scheduled message: seq=%d, bytes=%d, ID=%d\n", sequence_number,
         msg_len, id);

  sequence_number++;
  return next_job_run;
}

void AppInit() { ScheduleJob(MessageQueueExampleJob, ASAP()); }
