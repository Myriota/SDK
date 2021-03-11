// Copyright (c) 2020-2021, Myriota Pty Ltd, All Rights Reserved
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

// The simulation code for the at_modem example. This is only included when
// building the application for the host simulator.

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "at.h"

static uint8_t gnss_return = 0;  // 0-success, 1-fail

time_t KeepRFAwake() { return Never(); }

void LedTurnOn(void) {}
void LedTurnOff(void) {}
void LedToggle(void) {}

int GPIOSetModeOutput(uint8_t PinNum) { return 0; }
int GPIOSetHigh(uint8_t PinNum) { return 0; }
int GPIOSetLow(uint8_t PinNum) { return 0; }

void *UARTInit(UARTInterface UARTNum, uint32_t BaudRate, uint32_t Options) {
  return (void *)0xDEADBEEF;
}

void UARTDeinit(void *Handle) {}

int UARTWrite(void *Handle, const uint8_t *Tx, size_t Length) {
  fwrite(Tx, 1, Length, stderr);
  return 0;
}

int UARTRead(void *Handle, uint8_t *Rx, size_t Length) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  return read(STDIN_FILENO, Rx, Length);
}

int RFTestRxRSSI(int32_t *RSSI) {
  *RSSI = -90;
  return 0;
}

char *ModuleIDGet(void) { return "00749f046b M1-24"; }

char *RegistrationCodeGet(void) { return "g3z59x4e9frdt1j4ydmnb6jqy"; }

int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull) { return 0; }

#define GPIO_DONT_CARE GPIO_LOW
int GPIOGet(uint8_t PinNum) { return GPIO_DONT_CARE; }

int RFTestTxStart(uint32_t Frequency, uint8_t TxType, bool IsBurst) {
  if (Frequency < 142000000 || Frequency > 525000000 ||
      (Frequency > 175000000 && Frequency < 350000000)) {
    return -1;
  }
  return 0;
}

void RFTestTxStop(void) {}

int RFTestRxStart(uint32_t Frequency) {
  if (Frequency < 350000001 || Frequency > 420000000) {
    return -1;
  }
  return 0;
}

void RFTestRxStop(void) {}

int GNSSFix(void) { return gnss_return; }

time_t ScheduleHook(time_t Next) {
  raise(SIGUSR1);  // raise signal to simulate leuart activity
  return 0;        // return time of wakeup event
}
