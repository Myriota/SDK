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

// This example demonstrates uart interface communication between an external
// device and the Myriota terminal via the LEUART interface. This also
// demonstrates a basic modem application. Operates as follows:
// 1. the external device wakes up the terminal by setting GPIO0 high,
// 2. waits for the "READY\n" string from the terminal,
// 3. sends messsage string terminated by "\n",
// 4. sets GPIO0 low again.
// 5. The Myriota terminal acknowledges reception with string "\nOK\n",
// 6. then schedules the message string for satellite tramsmission.

#include <string.h>
#include "myriota_user_api.h"

const static uint8_t GPIOPin = PIN_GPIO0_WKUP;

#define READY_STRING "READY\n"
#define RECEIVE_TIMEOUT 2000  // [ms]
#define ACK_STRING "\nOK\n"

// Read new line terminated string from UART with timeout
// Return number of bytes read or -1 on timeout or string is too long
int UARTReadStringWithTimeout(void *Handle, uint8_t *Rx, size_t MaxLength) {
  const uint32_t start = TickGet();
  int count = 0;
  while (TickGet() - start < RECEIVE_TIMEOUT) {
    uint8_t ch;
    if (UARTRead(Handle, &ch, 1) == 1) {
      if (ch == '\n') return count;
      Rx[count] = ch;
      count++;
      if (count == MaxLength) return -1;
    }
  }
  return -1;
}

static time_t UartComm() {
  void *uart_handle = UARTInit(LEUART, 9600, 0);
  if (uart_handle == NULL) {
    printf("Failed to initialise leuart\n");
    return OnGPIOWakeup();
  }
  UARTWrite(uart_handle, (uint8_t *)READY_STRING, strlen(READY_STRING));

  uint8_t Rx[MAX_MESSAGE_SIZE];
  int len = UARTReadStringWithTimeout(uart_handle, Rx, MAX_MESSAGE_SIZE);
  if (len <= 0) {
    printf("Failed to receive message\n");
  } else {
    UARTWrite(uart_handle, (uint8_t *)Rx, len);
    UARTWrite(uart_handle, (uint8_t *)ACK_STRING, strlen(ACK_STRING));
    ScheduleMessage(Rx, len);
    printf("Scheduled message: ");
    for (int i = 0; i < len; i++) printf("%02x", Rx[i]);
    printf("\n");
  }

  UARTDeinit(uart_handle);
  return OnGPIOWakeup();
}

void AppInit() {
  // Internally pull down the pin to make sure the pin is not floating
  GPIOSetModeInput(GPIOPin, GPIO_PULL_DOWN);
  ScheduleJob(UartComm, OnGPIOWakeup());
  GPIOSetWakeupLevel(GPIOPin, GPIO_HIGH);
}
