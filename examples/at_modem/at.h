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

#define UART_MAX_TX_SIZE 60
#define UART_MAX_RX_SIZE 80
#define RECEIVE_TIMEOUT 200  // [ms]

#define RF_TX_TIMEOUT_MAX 999000  // [ms]
#define VHF_TX_DEFAULT_FREQUENCY 161450000
#define UHF_TX_DEFAULT_FREQUENCY 400000000
#define RF_RX_DEFAULT_FREQUENCY 400000000

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

int ATReceive(char *Rx, size_t MaxLength);

void ATProcess(char *Start, int Len);

time_t KeepRFAwake();

#endif
