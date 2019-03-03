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

// Hardware-specific user programming interface for the Myriota terminal

#ifndef MYRIOTA_HARDWARE_API_H
#define MYRIOTA_HARDWARE_API_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "bsp.h"
#include "module_pindefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// On board temperature sensor
// Get on board temperature in degrees Celsius
// Returns 0 if succeeded and -1 if failed
int TemperatureGet(float *Temperature);

/// LED
// Control the LED
void LedTurnOn(void);
void LedTurnOff(void);
void LedToggle(void);

/// ADCs
/// PinNum is pin number of the core module, can be PIN_ADC0 or PIN_ADC1
// Reference voltage, can be VIO_REF, 2.5V and 1.25V
typedef enum { ADC_REF_VIO = 0, ADC_REF_2V5, ADC_REF_1V25 } ADCReference;
// Get the converted voltage (mV) from the MCU ADC port
// Returns 0 if read succeeds, -1 if read fails
int ADCGetVoltage(uint8_t PinNum, ADCReference Reference, uint32_t *mV);
// Get raw value from the ADC port [0 - 0xFFF]
// Returns 0 if read succeeds, -1 if read fails
int ADCGetValue(uint8_t PinNum, ADCReference Reference, uint16_t *Value);

/// I2C master
// Initialise I2C bus
// Returns 0 if initialisation succeeds, -1 if initialisation fails
int I2CInit(void);
// Deinitialise I2C bus
void I2CDeinit(void);
// Write to an I2C device
// Returns 0 if write succeeds, -1 if read fails
int I2CWrite(uint16_t DeviceAddress, const uint8_t *Command,
             size_t CommandLength);
// Read from an I2C device
// Returns 0 if read succeeds, -1 if read fails
int I2CRead(uint16_t DeviceAddress, const uint8_t *Command,
            size_t CommandLength, uint8_t *Rx, size_t RxLength);

/// SPI master
enum {
  SPI_BAUDRATE_DEFAULT = 1000000,
  SPI_BAUDRATE_MAX = 24000000,
};

// Initialise SPI master
// Returns 0 if initialisation succeeds, -1 if initialisation fails
int SPIInit(uint32_t BaudRate);
// Deinitialise SPI master
void SPIDeinit(void);
// Write to an SPI device
// Returns 0 if write succeeds, -1 if read fails
int SPIWrite(const uint8_t *Tx, size_t Length);
// SPI data transfer
// Returns 0 if transfer succeeds, -1 if transfer fails
int SPITransfer(const uint8_t *Tx, uint8_t *Rx, size_t Length);

/// GPIOs
/// PinNum is pin number of the core module, can be any pins
// GPIO level
typedef enum { GPIO_LOW = 0, GPIO_HIGH } GPIOLevel;
// GPIO internal pull up/down
typedef enum { GPIO_NO_PULL = 10, GPIO_PULL_UP, GPIO_PULL_DOWN } GPIOPull;

// Set a GPIO to input mode and the internal pull up/down
// Returns 0 if succeeded and -1 if failed
int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull);
// Set a GPIO to output mode
// Returns 0 if succeeded and -1 if failed
int GPIOSetModeOutput(uint8_t PinNum);
// Set a GPIO output high
// Returns 0 if succeeded and -1 if failed
int GPIOSetHigh(uint8_t PinNum);
// Set a GPIO output low
// Returns 0 if succeeded and -1 if failed
int GPIOSetLow(uint8_t PinNum);
// Get input level of a GPIO
// Returns GPIOLevel if succeeded and -1 if failed
int GPIOGet(uint8_t PinNum);
// Set level polarity of a GPIO pin with wakeup capability for next wakeup
// Make sure to set the GPIO to input mode
// Returns 0 if succeeded and -1 if failed
int GPIOSetWakeupLevel(uint8_t PinNum, GPIOLevel Level);
// Disable wakeup capability of a GPIO pin
int GPIODisableWakeup(uint8_t PinNum);

/// UARTs
/// UARTNum is the UART number defined by the enum
// Enum to define the UART numbers
typedef enum { UART_0 = 0, UART_1, LEUART } UARTInterface;
// Initialise a uart interface, options are not supported and should be 0
// Returns the handle if succeeded and NULL if failed
// LEUART only supports baudrate up to 9600
void *UARTInit(UARTInterface UARTNum, uint32_t BaudRate, uint32_t Options);
// Deintialise a UART interface
void UARTDeinit(void *Handle);
// Write to a UART interface
// Returns 0 if write succeeds, -1 if read fails
int UARTWrite(void *Handle, const uint8_t *Tx, size_t Length);
// Read from to a UART interface non-blockingly, buffered
// Returns number of bytes read back, -1 if read fails
int UARTRead(void *Handle, uint8_t *Rx, size_t Length);

/// Battery voltage
// Returns 0 if succeeded and -1 if failed
int BatteryGetVoltage(uint32_t *mV);

/// System tick
// Get the current system tick (1000 ticks per second)
uint32_t TickGet(void);

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_HARDWARE_API_H
