// Copyright (c) 2020, Myriota Pty Ltd, All Rights Reserved
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

// This example demonstrates how to use Myriota module as satellite modem
// controlled by AT commands.

#include "at.h"

static time_t ModemReceive() {
  char rx[UART_MAX_RX_SIZE] = {0};
  int len = ATReceive(rx, UART_MAX_RX_SIZE);
  if (len > 0) {
    ATProcess(rx, len);
  } else {
    printf("No data received\n");
  }
  return OnLeuartReceive();
}

static time_t UARTReady() {
  if (HasValidGNSSFix()) {
    ATSetState(SYS_STATE_READY);
    printf("State = READY\n");
  } else {
    ATSetState(SYS_STATE_GNSS_ACQ);
    printf("State = GNSS_ACQ\n");
  }
  LedTurnOn();
  Delay(100);
  LedTurnOff();
  return Never();
}

void AppInit() {
  ScheduleJob(UARTReady, ASAP());
  ScheduleJob(ModemReceive, OnLeuartReceive());
}

int BoardStart(void) {
  LedTurnOn();
  Delay(100);
  LedTurnOff();
  if (ATInit())
    printf("Failed init modem\n");
  else {
    ATSetState(SYS_STATE_INIT);
    printf("Myriota modem example\n");
  }
  return 0;
}
