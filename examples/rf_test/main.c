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

// This example demonstrates how to use RF test command.
//
// Like the blinky example, this example runs in sandbox mode.

#include <unistd.h>
#include "myriota_user_api.h"

#define MODULE_BAND_PIN PIN_BAND
#define ANT_SEL_PIN PIN_GPIO6

#define VHF_TX_DEFAULT_FREQUENCY 161450000
#define UHF_TX_DEFAULT_FREQUENCY 400000000

void AppInit() {}

int BoardStart() {
  GPIOSetModeInput(MODULE_BAND_PIN, GPIO_NO_PULL);
  uint32_t TxFreq;
  if (GPIOGet(MODULE_BAND_PIN) == GPIO_HIGH) {
    printf("Testing VHF module %s\n", ModuleIDGet());
    TxFreq = VHF_TX_DEFAULT_FREQUENCY;
  } else {
    printf("Testing UHF module %s\n", ModuleIDGet());
    TxFreq = UHF_TX_DEFAULT_FREQUENCY;
  }
  while (1) {
    if (RFTestTxStart(TxFreq, TX_TYPE_TONE, false)) {
      printf("Failed to start\n");
    } else {
      printf("Test started\n");
      Delay(10000);
      RFTestTxStop();
      printf("Test stopped\n");
    }
    printf("Press any key to continue\n");
    char Ch;
    while (!read(0, &Ch, 1))
      ;
  }
}

// Override antenna selection logic in BSP
// For Myriota development board, use external antenna for RF tests
int BoardAntennaSelect(RadioMode Mode, RadioBand Band) {
  GPIOSetModeOutput(ANT_SEL_PIN);
  GPIOSetLow(ANT_SEL_PIN);
  return 0;
}
