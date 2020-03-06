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

#include "myriota_user_api.h"

static int led_state = 0;

void LedTurnOn(void) {
  led_state = 1;
  printf("%ld %s Led=%d\n", time(NULL), __func__, led_state);
}

void LedTurnOff(void) {
  led_state = 0;
  printf("%ld %s Led=%d\n", time(NULL), __func__, led_state);
}

void LedToggle(void) {
  led_state = led_state ? 0 : 1;
  printf("%ld %s Led=%d\n", time(NULL), __func__, led_state);
}
