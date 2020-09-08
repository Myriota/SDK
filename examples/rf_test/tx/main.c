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

// This example demonstrates how to use transmit related RF test API.
//
// Like the blinky example, this example runs in sandbox mode.

// This example can be used to do RF TX power, current draw and RF path
// connectivity checks as well. More information can be found in README file

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
    printf("Testing radio transmit, press any key to stop\n");
    if (RFTestTxStart(TxFreq, TX_TYPE_TONE, false)) {
    } else {
      char Ch;
      uint32_t TickStart = TickGet();
      while (!read(0, &Ch, 1) && TickGet() - TickStart < 10 * 1000)
        ;
      RFTestTxStop();
      printf("Tx test stopped\n");
    }
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
