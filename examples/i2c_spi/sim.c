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

// The simulation code for the event example. This is only included when
// building the application for the simulator platform.

#include "LIS3DH_defs.h"
#include "myriota_user_api.h"

#ifdef USING_I2C
static int16_t x = -2, y = 3, z = 16383;
#else
static int16_t x = -3, y = 4, z = 16384;
#endif

static uint8_t LastCommand = UINT8_MAX;
int I2CInit() { return 0; }
int I2CInitEx(uint32_t Option) { return 0; }
void I2CDeinit(void) {}
int I2CWrite(uint16_t DeviceAddress, const uint8_t *Command,
             size_t CommandLength) {
  LastCommand = *Command;
  return 0;
}
int I2CRead(uint16_t DeviceAddress, const uint8_t *Command,
            size_t CommandLength, uint8_t *Rx, size_t RxLength) {
  uint8_t Cmd;
  if (CommandLength == 0) {
    Cmd = LastCommand;
  } else {
    Cmd = *Command;
  }
  switch (Cmd) {
    case LIS3DH_REG_CTRL1:
      *Rx = 0x8;
      break;
    case LIS3DH_REG_WHOAMI:
      *Rx = 0x33;
      break;
    case LIS3DH_REG_OUT_X_L | 0x80:
      Rx[0] = x & 0xFF;
      Rx[1] = x >> 8;
      Rx[2] = y & 0xFF;
      Rx[3] = y >> 8;
      Rx[4] = z & 0xFF;
      Rx[5] = z >> 8;
      break;
    default:
      *Rx = 0;
      break;
  }
  return 0;
}

int SPIInit(uint32_t BaudRate) { return 0; }
void SPIDeinit(void) {}
int SPIWrite(const uint8_t *Tx, size_t Length) { return 0; }
int SPITransfer(const uint8_t *Tx, uint8_t *Rx, size_t Length) {
  switch (*Tx) {
    case LIS3DH_REG_CTRL1 | 0x80:
      Rx[1] = 0x8;
      break;
    case LIS3DH_REG_WHOAMI | 0x80:
      Rx[1] = 0x33;
      break;
    case LIS3DH_REG_OUT_X_L | 0x80 | 0x40:
      Rx[1] = x & 0xFF;
      Rx[2] = x >> 8;
      Rx[3] = y & 0xFF;
      Rx[4] = y >> 8;
      Rx[5] = z & 0xFF;
      Rx[6] = z >> 8;
      break;
    default:
      *Rx = 0;
      break;
  }
  return 0;
}
