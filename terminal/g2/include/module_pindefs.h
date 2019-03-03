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

// I/O pins of the module

#ifndef MODULE_PINDEFS_H
#define MODULE_PINDEFS_H

enum {
  PIN_BAND = 2,
  PIN_ADC1 = 6,
  PIN_ADC0 = 8,
  PIN_PULSE1 = 10,
  PIN_PULSE0 = 11,
  PIN_UART0_RTS = 12,
  PIN_GPIO2 = 14,
  PIN_GPIO3 = 15,
  PIN_RF_EN = 17,
  PIN_SPI_CS = 19,
  PIN_SPI_SCK = 20,
  PIN_SPI_MISO = 21,
  PIN_SPI_MOSI = 22,
  PIN_GPIO0_WKUP = 24,
  PIN_RF_TEST2 = 32,
  PIN_GPIO4 = 33,
  PIN_GPIO5 = 34,
  PIN_GPIO6 = 35,
  PIN_GPIO1_WKUP = 41,
  PIN_USB_D_P = 45,
  PIN_USB_D_N = 46,
  PIN_UART1_RX = 48,
  PIN_UART1_TX = 49,
  PIN_I2C_SCL = 50,
  PIN_I2C_SDA = 51,
  PIN_GPIO8 = 52,
  PIN_GPIO7 = 53,
  PIN_UART0_CTS = 54,
  PIN_UART0_TX = 55,
  PIN_UART0_RX = 56,
  PIN_LEUART_RX = 57,
  PIN_LEUART_TX = 58,
  PIN_MAX
};

#endif /* MODULE_PINDEFS_H */
