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

// The simulation code for the snl example. This is only included when
// building the application for the simulator platform.

#include <signal.h>
#include "myriota_user_api.h"

static const uint32_t voltage_list[6] = {19, 20, 359, 360, 2200, 2201};

static uint8_t pin_state = GPIO_LOW;  // Current pin state

int GPIOSetHigh(uint8_t PinNum) { return 0; }

int GPIOSetLow(uint8_t PinNum) { return 0; }

int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull) { return 0; }

int GPIOSetWakeupLevel(uint8_t PinNum, GPIOLevel Level) { return 0; }

int GPIOSetModeOutput(uint8_t PinNum) { return 0; }

void LedTurnOn(void) {}

void LedTurnOff(void) {}

int GPIOGet(uint8_t PinNum) { return pin_state; }

int ADCGetVoltage(uint8_t PinNum, ADCReference Reference, uint32_t *mV) {
  static uint8_t i = 0;
  *mV = voltage_list[i++];
  if (i > 5) i = 0;
  return 0;
}

// Generate GPIO wakeup event
time_t ScheduleHook(time_t Next) {
  const time_t next_event = TimeGet() + 5;

  // wakeup time is before the next, return 0 to indicate no event
  if (next_event >= Next) return 0;

  if (pin_state == GPIO_LOW)
    pin_state = GPIO_HIGH;
  else
    pin_state = GPIO_LOW;
  raise(SIGUSR1);     // raise signal to indicate wakeup event
  return next_event;  // return time of wakeup event
}
