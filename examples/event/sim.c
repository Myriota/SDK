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

// The simulation code for the event example. This is only included when
// building the application for the simulator platform.

#include <signal.h>
#include <unistd.h>
#include "myriota_user_api.h"

// Run "kill -l 10 processid" to generate events. The processid is
// printed when the simulation starts

static uint8_t pin_state = GPIO_LOW;  // Current pin state

int GPIOSetWakeupLevel(uint8_t PinNum, GPIOLevel Level) { return 0; }

int GPIOGet(uint8_t PinNum) { return pin_state; }

int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull) { return 0; }

// Generate the GPIO wakeup event every hour
time_t ScheduleHook(time_t Next) {
  // current time rounded up to the next hour
  const time_t next_event = (TimeGet() / 3600 + 1) * 3600;

  // wakeup time is before the next hour return 0 to indicate no event
  if (next_event >= Next) return 0;

  // wakeup on the next hour
  if (pin_state == GPIO_LOW)
    pin_state = GPIO_HIGH;
  else
    pin_state = GPIO_LOW;
  raise(SIGUSR1);     // raise signal to indicate wakeup event
  return next_event;  // return time of wakeup event
}
