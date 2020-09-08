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

// This example demonstrates how to test GNSS via the debug port.
// All input and output of the GNSS chip is relayed via debug port for testing,
// e.g. GNSS chip vendor's tool can be connected to test performance.
//
// Like the blinky example, this example runs in sandbox mode.

#include "myriota_user_api.h"

#define GNSS_DEFAULT_BAUDRATE 9600

void AppInit() {}

int BoardStart() {
  void *uart0_handle, *uart1_handle;

  // Repurpose the debug port
  BoardDebugDeinit();
  uart0_handle = UARTInit(UART_0, GNSS_DEFAULT_BAUDRATE, 0);

  // Configure the GNSS UART port
  uart1_handle = UARTInit(UART_1, GNSS_DEFAULT_BAUDRATE, 0);
  BoardGNSSPowerEnable();

  // Replay GNSS traffic
  while (1) {
    uint8_t Ch;
    while (1 == UARTRead(uart1_handle, &Ch, 1)) {
      UARTWrite(uart0_handle, &Ch, 1);
    }
    while (1 == UARTRead(uart0_handle, &Ch, 1)) {
      UARTWrite(uart1_handle, &Ch, 1);
    }
  }
  return 0;
}
