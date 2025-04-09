// Copyright (c) 2016-2023, Myriota Pty Ltd, All Rights Reserved
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

// An example that receives messages from space using the OnReceiveMessage event
// and the ReceiveMessage API. The example periodically schedules messages
// containing the current time, the number of messages received to date, as well
// as the time and first 10 bytes of the most recently received message.
// Additional messages are scheduled when an OnReceiveMessage event triggers.

#include <string.h>
#include "myriota_user_api.h"

#define MESSAGE_PER_DAY 3

// Format of transmit message. Values are little Endian.
typedef struct {
  uint32_t time;           // epoch timestamp of scheduling this message
  uint16_t count_rx;       // number of receive messages
  uint32_t time_rx;        // epoch timestamp of most recent receive message
  uint8_t message_rx[10];  // content of most recent receive message,
                           // potentially truncated
} __attribute__((packed)) transmit_message;

static transmit_message tx_msg = {0, 0, 0, {0}};

static void TransmitMessageInit(void) {
  // invalidate receive time and message content with all-cc pattern
  tx_msg.time_rx = 0xcccccccc;
  memset(tx_msg.message_rx, 0xcc, sizeof(tx_msg.message_rx));
}

static time_t ReceiveJob(void) {
  int size;
  const uint8_t *rx_msg = ReceiveMessage(&size);
  if (size <= 0)
    return OnReceiveMessage();  // nothing here, wait for next event

  // new receive message, update transmit message content
  tx_msg.count_rx++;
  tx_msg.time_rx = TimeGet();
  const int echo_size =
      size < sizeof(tx_msg.message_rx) ? size : sizeof(tx_msg.message_rx);
  memset(tx_msg.message_rx, 0xcc, sizeof(tx_msg.message_rx));
  memcpy(tx_msg.message_rx, rx_msg, echo_size);

  printf("%" PRIu32 " Received new message: %.*s (%" PRIu16
         " messages to date)\n",
         tx_msg.time_rx, size, rx_msg, tx_msg.count_rx);

  tx_msg.time = TimeGet();
  ScheduleMessage((void *)&tx_msg, sizeof(tx_msg));

  printf("%" PRIu32 " Scheduled message from ReceiveJob: count_rx=%" PRIu16
         "\n",
         tx_msg.time, tx_msg.count_rx);

  return OnReceiveMessage();  // wait for next event
}

static time_t TransmitJob(void) {
  const time_t now = TimeGet();

  tx_msg.time = TimeGet();
  ScheduleMessage((void *)&tx_msg, sizeof(tx_msg));

  printf("%" PRIu32 " Scheduled message from TransmitJob: count_rx=%" PRIu16
         "\n",
         tx_msg.time, tx_msg.count_rx);

  return now + 24 * 3600 / MESSAGE_PER_DAY;
}

void AppInit() {
  TransmitMessageInit();
  ScheduleJob(TransmitJob, ASAP());
  ScheduleJob(ReceiveJob, OnReceiveMessage());
}
