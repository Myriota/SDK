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

// This example demonstrates how to communicate with the ST LIS3DH, a 3-axis
// linear accelerometer, over the I2C/SPI interface.
//
// The LIS3DH is a popular low power triple-axis accelerometer. It has both I2C
// and SPI interface options. This example uses a breakout board from Adafruit:
// https://www.adafruit.com/product/2809.
// The datasheet and other resources can be found here:
// https://learn.adafruit.com/adafruit-lis3dh-triple-axis-accelerometer-breakout/downloads
//
// Like the blinky example, this example runs in sandbox mode.
//
// I2C wiring:
//  __________________________________
// | LIS3DH board | Myriota dev board |
//  ==================================
// |     Vin     <--     VEXT         |
// |     GND     <--     GND          |
// |     SCL     <--    I2C_SCL       |
// |     SDA     <-->   I2C_SDA       |
// |     SDO     <--    See below     |
// |----------------------------------|
// SDO - When in I2C mode, this pin can be used for I2C address selection:
// When connected to GND or left open, the address is 0x18;
// When connected to 3.3V, the address is 0x19
//
// SPI wiring:
//  __________________________________
// | LIS3DH board | Myriota dev board |
//  ==================================
// |     Vin     <--     VEXT         |
// |     GND     <--     GND          |
// |     SCL     <--    SPI_SCK       |
// |     SDA     <--    SPI_MOSI      |
// |     SDO     -->    SPI_MISO      |
// |     CS      <--    SPI_CS        |
// |----------------------------------|
//

#include "LIS3DH_defs.h"
#include "myriota_user_api.h"

#define SPI_BAUDRATE_DIVIDER 24  // Integer from 1 to 8000
#define SPI_BAUDRATE (SPI_BAUDRATE_MAX / SPI_BAUDRATE_DIVIDER + 1)

static int16_t x, y, z;

#ifdef USING_I2C
static int ReadRegister8(uint8_t reg) {
  uint8_t rx;
  if (I2CInit() == 0 &&
      I2CRead(LIS3DH_I2C_ADDRESS, &reg, sizeof(reg), &rx, sizeof(rx)) == 0) {
    I2CDeinit();
    return rx;
  }

  return -1;
}

static int WriteRegister8(uint8_t reg, uint8_t value) {
  uint8_t tx[2];
  tx[0] = reg;
  tx[1] = value;
  if (I2CInit() == 0 && I2CWrite(LIS3DH_I2C_ADDRESS, tx, sizeof(tx)) == 0) {
    I2CDeinit();
  } else {
    return -1;
  }

  return 0;
}

static int ReadSample(void) {
  uint8_t rx[LIS3DH_SAMPLE_SIZE];
  uint8_t reg = LIS3DH_REG_OUT_X_L | 0x80;
  if (I2CInitEx(I2C_FAST_MODE) == 0) {
    I2CWrite(LIS3DH_I2C_ADDRESS, &reg, sizeof(reg));
    I2CRead(LIS3DH_I2C_ADDRESS, NULL, 0, rx, sizeof(rx));
    I2CDeinit();
    x = (rx[1] << 8) | rx[0];
    y = (rx[3] << 8) | rx[2];
    z = (rx[5] << 8) | rx[4];
    return 0;
  }

  I2CDeinit();
  return -1;
}

#else
static int ReadRegister8(uint8_t reg) {
  uint8_t rx[2];  // 1 byte to skip reg addr and 1 byte for data
  // Set bit 7 to read
  reg |= 0x80;
  if (SPIInit(SPI_BAUDRATE) == 0 && SPITransfer(&reg, rx, sizeof(rx)) == 0) {
    SPIDeinit();
    return rx[1];
  }

  return 0;
}

static int WriteRegister8(uint8_t reg, uint8_t value) {
  uint8_t tx[2];  // 1 byte to skip reg addr and 1 byte for data
  // Clear bit 7 to write
  reg &= ~0x80;
  tx[0] = reg;
  tx[1] = value;

  if (SPIInit(SPI_BAUDRATE) == 0 && SPIWrite(tx, sizeof(tx)) == 0) {
    SPIDeinit();
  } else {
    return -1;
  }

  return 0;
}

static int ReadSample(void) {
  uint8_t rx[1 + LIS3DH_SAMPLE_SIZE];  // 1 byte to skip reg addr and the rest
                                       // for sample data
  uint8_t reg = LIS3DH_REG_OUT_X_L | 0x80 | 0x40;
  if (SPIInit(SPI_BAUDRATE) == 0 && SPITransfer(&reg, rx, sizeof(rx)) == 0) {
    SPIDeinit();
    x = (rx[2] << 8) | rx[1];
    y = (rx[4] << 8) | rx[3];
    z = (rx[6] << 8) | rx[5];
    return 0;
  }
  SPIDeinit();
  return -1;
}
#endif

static int SetDataRate(Lis3dhDataRate data_rate) {
  uint8_t ctl1 = ReadRegister8(LIS3DH_REG_CTRL1);
  ctl1 &= ~(0xF0);
  ctl1 |= (data_rate << 4);
  if (WriteRegister8(LIS3DH_REG_CTRL1, ctl1) != 0) {
    return -1;
  }

  return 0;
}

// Setup the HW
static int Init(void) {
  uint8_t deviceid = ReadRegister8(LIS3DH_REG_WHOAMI);
  if (deviceid != 0x33) {
    // No sensor detected
    return -1;
  }

  // Enable all axes, normal mode
  if (WriteRegister8(LIS3DH_REG_CTRL1, 0x07) != 0) {
    return -1;
  }

  // 400Hz rate
  if (SetDataRate(LIS3DH_DATARATE_400_HZ) != 0) {
    return -1;
  }

  return 0;
}

void AppInit() {}

int BoardStart() {
  if (Init() == 0) {
#ifdef USING_I2C
    printf("I2C communication is OK\n");
#else
    printf("SPI communication is OK\n");
#endif
    while (1) {
      if (ReadSample() == 0) {
        printf("x = %d, y = %d, z = %d\n", x, y, z);
      } else {
        printf("Failed to read from the sensor!\n");
      }

      Sleep(2);
    }
  } else {
    printf("Failed to communicate with the sensor!\n");
  }

  return -1;
}
