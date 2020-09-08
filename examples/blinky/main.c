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

// Demonstrates putting the Myriota Module in "sandbox" by overriding the
// BoardStart() API. Also demonstrates how to control the LED.
//
// Sandbox mode is useful for development because system initialisation is
// faster. Sandbox mode is not for deployment. In this mode the module will
// not transmit data to satellite.

#include "myriota_user_api.h"

void AppInit() {}

int BoardStart() {
  while (1) {
    LedTurnOn();
    printf("Led On\n");
    Sleep(1);
    LedTurnOff();
    printf("Led Off\n");
    Sleep(1);
    LedToggle();
    printf("Led On\n");
    Sleep(1);
    LedTurnOff();
    printf("Led Off\n");
    Sleep(1);
  }
}
