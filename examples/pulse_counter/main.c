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

// Can be tested with development board by shorting pin27(GPIO0_WKUP) and
// pin30(PULSE0) on the breakout header and pressing the WAKEUP button

#include "myriota_user_api.h"

static time_t RunsOnPulseCounterEvent() {
  printf("Woken up by Pulse Counter @ %d\n", (int)TimeGet());

  printf("Current pulse counter value: %" PRIu32 "\n",
         (uint32_t)PulseCounterGet());

  return OnPulseCounterEvent();
}

void AppInit() {
  // Initialise to generate event every 6 pulses
  if (PulseCounterInit(6, 0)) printf("Failed to initialise pulse counter\n");

  // Schedule a job to process pulse counter event
  ScheduleJob(RunsOnPulseCounterEvent, OnPulseCounterEvent());
}
