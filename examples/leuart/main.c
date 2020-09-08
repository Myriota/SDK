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

// This example demonstrates leuart interface communication between an external
// device and the Myriota device via the LEUART interface. Myriota device can be
// woken up when data is received from LEUART. This also demonstrates a basic
// modem application. Operates as follows:
// 1. the external device sends messsage string,
// 2. the Myriota device wakes up and echoes back the message string,
// 3. the Myriota device sends "\nOK\n",
// 4. then schedules the message string for satellite transmission.

#include <string.h>
#include "myriota_user_api.h"

#define RECEIVE_TIMEOUT 100  // [ms]
#define ACK_STRING "\nOK\n"
#define OVERLOAD_STRING "\nOVERLOADED\n"

static void *leuart_handle = NULL;

// Read string from UART with timeout, return number of bytes read
int UARTReadStringWithTimeout(void *Handle, uint8_t *Rx, size_t MaxLength) {
  const uint32_t start = TickGet();
  int count = 0;
  while (TickGet() - start < RECEIVE_TIMEOUT) {
    uint8_t ch;
    if (UARTRead(Handle, &ch, 1) == 1) {
      if (count < MaxLength) {
        Rx[count] = ch;
      }
      count++;
    }
  }
  return count;
}

static time_t UartComm() {
  uint8_t Rx[MAX_MESSAGE_SIZE] = {0};
  int len = UARTReadStringWithTimeout(leuart_handle, Rx, MAX_MESSAGE_SIZE);
  if (len > MAX_MESSAGE_SIZE) {
    printf("LEUART RX buffer overloaded\n");
    UARTWrite(leuart_handle, (uint8_t *)OVERLOAD_STRING,
              strlen(OVERLOAD_STRING));
  } else if (len > 0) {
    UARTWrite(leuart_handle, (uint8_t *)Rx, len);
    UARTWrite(leuart_handle, (uint8_t *)ACK_STRING, strlen(ACK_STRING));
    ScheduleMessage(Rx, len);
    printf("Scheduled message: ");
    for (int i = 0; i < len; i++) printf("%02x", Rx[i]);
    printf("\n");
  } else {
    printf("No data received\n");
  }
  return OnLeuartReceive();
}

void AppInit() {
  ScheduleJob(UartComm, OnLeuartReceive());
  leuart_handle = UARTInit(LEUART, 9600, 0);
  if (leuart_handle == NULL) {
    printf("Failed to initialise leuart\n");
    return;
  }
}
