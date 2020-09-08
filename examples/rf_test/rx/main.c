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

// This example demonstrates how to use receive related RF test API.
// Further information can be found in the README file
//
// Like the blinky example, this example runs in sandbox mode.

#include <unistd.h>
#include "myriota_user_api.h"

#define MODULE_BAND_PIN PIN_BAND
#define ANT_SEL_PIN PIN_GPIO6

#define RX_DEFAULT_FREQUENCY 400500000

void AppInit() {}

int BoardStart() {
  GPIOSetModeInput(MODULE_BAND_PIN, GPIO_NO_PULL);
  if (GPIOGet(MODULE_BAND_PIN) == GPIO_HIGH) {
    printf("Testing VHF module %s\n", ModuleIDGet());
  } else {
    printf("Testing UHF module %s\n", ModuleIDGet());
  }
  while (1) {
    printf("Testing radio receive, press any key to stop\n");
    RFTestRxStart(RX_DEFAULT_FREQUENCY);
    while (1) {
      int32_t RSSI;
      if (RFTestRxRSSI(&RSSI)) {
        printf("Failed to read RSSI\n");
      } else {
        printf("RSSI = %ddBm\n", (signed)RSSI);
        // LogAdd(0, &RSSI, sizeof(RSSI));
      }
      char Ch;
      if (1 == read(0, &Ch, 1)) {
        break;
      }
      Delay(1000);
    }
    RFTestRxStop();
    printf("Rx test stopped\n");
    printf("Press any key to continue testing\n");
    char Ch;
    while (!read(0, &Ch, 1))
      ;
  }
  return 0;
}

// Use RF test specific antenna selection logic
int BoardAntennaSelect(RadioMode Mode, RadioBand Band) {
  // Set the antenna select pin to proper state to save power
  if ((Mode == RADIO_MODE_INIT) || (Mode == RADIO_MODE_DEINIT)) {
    GPIOSetLow(ANT_SEL_PIN);
    return 0;
  }

  bool UseUHFAntenna;
  if (Band == RADIO_BAND_VHF) {
    UseUHFAntenna = false;
  } else {
    UseUHFAntenna = true;
  }

  GPIOSetModeOutput(ANT_SEL_PIN);

  if (UseUHFAntenna) {
    GPIOSetHigh(ANT_SEL_PIN);
  } else {
    GPIOSetLow(ANT_SEL_PIN);
  }
  return 0;
}
