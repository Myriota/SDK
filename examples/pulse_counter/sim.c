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

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "myriota_user_api.h"

// Run "kill -l 10 processid" to generate events. The processid is
// printed when the simulation starts
//

static uint32_t PulseCounterLimit = 0;
static uint32_t PulseCounterOptions = 0;
static bool PulseCounterInited = false;
uint64_t Counter = 0;

int PulseCounterInit(uint32_t Limit, uint32_t Options) {
  if (PulseCounterInited) {
    return -1;
  }

  PulseCounterLimit = Limit;
  PulseCounterOptions = Options;
  PulseCounterInited = true;
  return 0;
}

uint64_t PulseCounterGet() {
  if (!PulseCounterInited) return 0;

  return Counter;
}

int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull) { return 0; }

// Generate the pulse counter event every hour
time_t ScheduleHook(time_t Next) {
  time_t WakeupInterval = 3600;

  // current time rounded up to the next hour
  const time_t next_event = (time(NULL) / WakeupInterval + 1) * WakeupInterval;

  // wakeup time is before the next hour return 0 to indicate no event
  if (next_event >= Next) return 0;

  Counter += PulseCounterLimit;
  raise(SIGUSR1);     // raise signal to indicate wakeup event
  return next_event;  // return time of wakeup event
}
