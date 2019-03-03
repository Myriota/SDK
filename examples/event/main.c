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

// RunsOnGPIOWakeup runs when the GPIO wakeup pin is pressed or released.

#include "myriota_user_api.h"

const static uint8_t GPIOPin = PIN_GPIO0_WKUP;

static time_t RunsOnGPIOWakeup() {
  printf("Woken up by GPIO @ %d\n", (int)time(NULL));

  printf("GPIO%u level is %s\n", (unsigned int)GPIOPin,
         ((GPIOGet(GPIOPin) == GPIO_HIGH) ? "high" : "low"));

  // Change wakeup level so that we do not repeatedly wake from the same source.
  if (GPIOGet(GPIOPin) == GPIO_HIGH)
    GPIOSetWakeupLevel(GPIOPin, GPIO_LOW);
  else
    GPIOSetWakeupLevel(GPIOPin, GPIO_HIGH);

  return OnGPIOWakeup();
}

void AppInit() {
  // Internally pull down the pin to make sure the pin is not floating
  GPIOSetModeInput(GPIOPin, GPIO_PULL_DOWN);
  ScheduleJob(RunsOnGPIOWakeup, OnGPIOWakeup());
  GPIOSetWakeupLevel(GPIOPin, GPIO_HIGH);
}
