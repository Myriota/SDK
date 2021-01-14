// Copyright (c) 2019, Myriota Pty Ltd, All Rights Reserved
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

// The simulation code for the uart example. This is only included when
// building the application for the simulator platform. This simulator schedules
// a message for transmission every 8 hours.

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "myriota_user_api.h"

void *UARTInit(UARTInterface UARTNum, uint32_t BaudRate, uint32_t Options) {
  return (void *)0xDEADBEEF;
}
void UARTDeinit(void *Handle) {}

int UARTWrite(void *Handle, const uint8_t *Tx, size_t Length) {
  fwrite(Tx, 1, Length, stdout);
  return 0;
}

int UARTRead(void *Handle, uint8_t *Rx, size_t Length) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  return read(STDIN_FILENO, Rx, Length);
}

#define EIGHT_HOURS (8 * 60 * 60)

// Wake modem every 8 hours and schedule a message
time_t ScheduleHook(time_t Next) {
  const time_t next_event = (TimeGet() / EIGHT_HOURS + 1) * EIGHT_HOURS;

  if (next_event >= Next) return 0;  // zero to indicate no event

  raise(SIGUSR1);     // raise signal to simulate leuart activity
  return next_event;  // return time of wakeup event
}
