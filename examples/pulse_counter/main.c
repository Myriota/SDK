// Copyright (c) 2016-2019, Myriota Pty Ltd, All Rights Reserved
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

#include "myriota_user_api.h"

static time_t RunsOnPulseCounterEvent() {
  printf("Woken up by Pulse Counter @ %d\n", (int)time(NULL));

  printf("Current pulse counter value: %" PRIu32 "\n",
         (uint32_t)PulseCounterGet());

  return OnPulseCounterEvent();
}

void AppInit() {
  // Initialise to generate event every 6 pulses.
  if (PulseCounterInit(6, 0))
    printf("Failed to initialise pulse counter\n");

  // Pullup on pulse counter input.
  GPIOSetModeInput(PIN_PULSE0, GPIO_PULL_UP);

  // Schedule a job to process pulse counter overflow event
  ScheduleJob(RunsOnPulseCounterEvent, OnPulseCounterEvent());
}
