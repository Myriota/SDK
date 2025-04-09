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

#ifndef AT_H
#define AT_H

#include <stdlib.h>
#include <string.h>
#include "myriota_user_api.h"

#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
#endif

#ifndef NUM_ELEMS
#define NUM_ELEMS(array) (sizeof(array) / sizeof((array)[0]))
#endif

#define MODEM_TEST_STR "test"

#define MODEM_BUSY PIN_GPIO7  // Outputs high when a job is running

#define MODULE_BAND_PIN PIN_BAND

#define TICK_PER_SECOND 1000

#define UART_INTERFACE LEUART
#define UART_BAUDRATE 9600
// Large enough to hold AT_MAX_CMD_LEN, AT_MAX_PARA_LEN and sizeof("\r")
#define UART_MAX_RX_SIZE 3012
#define UART_MAX_TX_SIZE 3012
#define RECEIVE_TIMEOUT 100  // [ms]

#define RF_TX_TIMEOUT_MAX 999000  // [ms]
#define VHF_TX_DEFAULT_FREQUENCY 161450000
#define UHF_TX_DEFAULT_FREQUENCY 400000000
#define UHF_RX_DEFAULT_FREQUENCY 400500000

typedef void (*QueryHandlerFunc_t)(uint32_t);
typedef void (*ControlHandlerFunc_t)(uint32_t, const char *);

typedef enum {
  SYS_STATE_INIT = 0,
  SYS_STATE_GNSS_ACQ,
  SYS_STATE_READY,
} SysStates;

typedef struct {
  uint32_t Id;
  QueryHandlerFunc_t Handler;
} QueryHandler_t;

typedef struct {
  uint32_t Id;
  ControlHandlerFunc_t Handler;
} ControlHandler_t;

// Generic structure for all command types
typedef struct {
  uint32_t Id;
  void *Handler;
} CmdHandler_t;

void ATSetState(SysStates St);

int ATInit();

// Receive for maximum of RECEIVE_TIMEOUT ms, stop when isspace() is received
size_t ATReceiveTimeout(char *Rx, const size_t MaxLength);

// Receive for MaxLength chars, stop when
// * Buffer is full
// * Nothing received for RECEIVE_TIMEOUT ms
// * isspace() is received
size_t ATReceive(char *Rx, const size_t MaxLength);

void ATSend(const char *Tx);

void ATProcess(char *Start, const int Len);

time_t KeepRFAwake();

bool IsTestMode(const uint32_t timeout);

void HardwareTest();

#endif
