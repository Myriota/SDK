// Copyright (c) 2019-2022, Myriota Pty Ltd, All Rights Reserved
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

// This example demonstrates the handling of external IO wakeup event on rising
// or falling edge.
// RunsOnGPIOWakeup job is scheduled when wakeup button(PIN_GPIO0_WKUP) is
// pressed.

#include "myriota_user_api.h"

const static uint8_t GPIOPin = PIN_GPIO0_WKUP;

static time_t RunsOnGPIOWakeup() {
  printf("Woken up by GPIO @ %u\n", (unsigned int)TimeGet());

  printf("GPIO%u level is %s\n", (unsigned int)GPIOPin,
         ((GPIOGet(GPIOPin) == GPIO_HIGH) ? "high" : "low"));

  return OnGPIOWakeup();
}

void AppInit() {
  // Make sure the pin is not floating
  GPIOSetModeInput(GPIOPin, GPIO_PULL_DOWN);
  // Wake up on rising edge, GPIO_LOW means falling edge
  GPIOSetWakeupLevel(GPIOPin, GPIO_HIGH);
  ScheduleJob(RunsOnGPIOWakeup, OnGPIOWakeup());
}
