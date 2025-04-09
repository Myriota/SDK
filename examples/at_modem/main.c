// Copyright (c) 2020-2025, Myriota Pty Ltd, All Rights Reserved
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

// Build with DISABLE_GNSS_FIX=1 or change to 1 here to disable GNSS fix and use
// external time and location from host
#ifndef DISABLE_GNSS_FIX
#define DISABLE_GNSS_FIX 0
#endif

#define LED_DELAY 100               // ms
#define WAIT_FOR_TEST_TIMEOUT 3000  // ms

#if DISABLE_GNSS_FIX
char *BoardEnvGet() { return "GNSSFIX=0"; }
#endif

static time_t ModemReceive() {
  char RX[UART_MAX_RX_SIZE] = {0};
  int len = ATReceive(RX, UART_MAX_RX_SIZE);
  if (len > 0) {
    do {
      ATProcess(RX, len);
      len = ATReceive(RX, UART_MAX_RX_SIZE);
    } while (len > 0);
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
  Delay(LED_DELAY);
  LedTurnOff();
  return Never();
}

void AppInit() {
  ScheduleJob(UARTReady, ASAP());
  ScheduleJob(ModemReceive, OnLeuartReceive());
}

int BoardStart(void) {
  GPIOSetModeOutput(MODEM_BUSY);
  GPIOSetHigh(MODEM_BUSY);
  if (ATInit())
    printf("Failed to init modem\n");
  else {
    ATSetState(SYS_STATE_INIT);
    printf("Myriota modem example\n");
    LedTurnOn();
    if (IsTestMode(WAIT_FOR_TEST_TIMEOUT)) HardwareTest();
    LedTurnOff();
  }
  return 0;
}

void BoardSleepEnter(void) { GPIOSetLow(MODEM_BUSY); }

void BoardSleepExit(void) { GPIOSetHigh(MODEM_BUSY); }
