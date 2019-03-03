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

// Definitions of LIS3DH
#ifndef LIS3DH_DEFS_H
#define LIS3DH_DEFS_H

#define LIS3DH_I2C_ADDRESS (0x18)

#define LIS3DH_REG_WHOAMI (0x0F)
#define LIS3DH_REG_CTRL1 (0x20)
#define LIS3DH_REG_OUT_X_L (0x28)
#define LIS3DH_REG_OUT_X_H (0x29)
#define LIS3DH_REG_OUT_Y_L (0x2A)
#define LIS3DH_REG_OUT_Y_H (0x2B)
#define LIS3DH_REG_OUT_Z_L (0x2C)
#define LIS3DH_REG_OUT_Z_H (0x2D)

// Used with register 0x20 (LIS3DH_REG_CTRL1) to set data rate
typedef enum {
  LIS3DH_DATARATE_400_HZ = 0b0111,  //  400Hz
  LIS3DH_DATARATE_200_HZ = 0b0110,  //  200Hz
  LIS3DH_DATARATE_100_HZ = 0b0101,  //  100Hz
  LIS3DH_DATARATE_50_HZ = 0b0100,   //   50Hz
  LIS3DH_DATARATE_25_HZ = 0b0011,   //   25Hz
  LIS3DH_DATARATE_10_HZ = 0b0010,   //   10Hz
  LIS3DH_DATARATE_1_HZ = 0b0001,    //    1Hz
  LIS3DH_DATARATE_POWERDOWN = 0,
  LIS3DH_DATARATE_LOWPOWER_1K6HZ = 0b1000,
  LIS3DH_DATARATE_LOWPOWER_5KHZ = 0b1001,
} Lis3dhDataRate;

#define LIS3DH_SAMPLE_SIZE 6

#endif
