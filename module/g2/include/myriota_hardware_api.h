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

// Hardware-specific user programming interface for the Myriota terminal

#ifndef MYRIOTA_HARDWARE_API_H
#define MYRIOTA_HARDWARE_API_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup Hardware_interface_api Hardware interface API
/// Hardware interface related API
/// @{

/// @defgroup Module_pins Module pins
/// Module I/O pin definitions
/// @{

/// Module pin definitions
enum ModulePinDef {
  PIN_BAND = 2,  ///< read-only
  PIN_ADC1 = 6,
  PIN_ADC0 = 8,
  PIN_PULSE1 = 10,
  PIN_PULSE0 = 11,
  PIN_UART0_RTS = 12,
  PIN_GPIO2 = 14,
  PIN_GPIO3 = 15,
  PIN_RF_EN = 17,  ///< read-only
  PIN_SPI_CS = 19,
  PIN_SPI_SCK = 20,
  PIN_SPI_MISO = 21,
  PIN_SPI_MOSI = 22,
  PIN_GPIO0_WKUP = 24,
  PIN_GPIO4 = 33,
  PIN_GPIO5 = 34,
  PIN_GPIO6 = 35,
  PIN_GPIO1_WKUP = 41,
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

/// @}

/// @defgroup ADC ADC
/// PinNum can be PIN_ADC0 or PIN_ADC1.
/// @{

/// Reference voltages
typedef enum {
  ADC_REF_VIO = 0,  ///< use VIO_REF as reference voltage
  ADC_REF_2V5,      ///< use 2.5V as reference voltage
  ADC_REF_1V25      ///< use 1.25V as reference voltage
} ADCReference;
/// Get the converted voltage (mV) from the MCU ADC port.
/// Returns 0 if read succeeds and -1 if read fails.
int ADCGetVoltage(uint8_t PinNum, ADCReference Reference, uint32_t *mV);
/// Get raw value from the ADC port (0 - 0xFFF).
/// Returns 0 if read succeeds and -1 if read fails.
int ADCGetValue(uint8_t PinNum, ADCReference Reference, uint16_t *Value);

/// @}

/// @defgroup I2C_master I2C master
/// @{

/// I2C options, can be ORed
enum I2COption {
  I2C_DEFAULT_OPTIONS = 0,
  I2C_FAST_MODE = 1 << 0,  ///< ~400 kbit/s, default standard mode (~100 kbit/s)
};
/// Initialise I2C bus.
/// Returns 0 if initialisation succeeds and -1 if initialisation fails.
int I2CInit(void);
/// Initialise I2C bus with options.
/// Returns 0 if initialisation succeeds and -1 if initialisation fails.
int I2CInitEx(uint32_t Option);
/// Deinitialise I2C bus.
void I2CDeinit(void);
/// Write to an I2C device synchronously.
/// Returns 0 if write succeeds and -1 if read fails.
int I2CWrite(uint16_t DeviceAddress, const uint8_t *Command,
             size_t CommandLength);
/// Read from an I2C device.
/// Returns 0 if read succeeds and -1 if read fails.
int I2CRead(uint16_t DeviceAddress, const uint8_t *Command,
            size_t CommandLength, uint8_t *Rx, size_t RxLength);

/// @}

/// @defgroup SPI_master SPI master
/// @{

/// SPI baudrate
enum SPIBaudrate {
  SPI_BAUDRATE_DEFAULT = 1000000,  ///< default SPI baudrate
  SPI_BAUDRATE_MAX = 24000000      ///< maximum SPI baudrate
};
/// Initialise SPI master.
/// Returns 0 if initialisation succeeds and -1 if initialisation fails.
int SPIInit(uint32_t BaudRate);
/// Deinitialise SPI master.
void SPIDeinit(void);
/// Write to an SPI device synchronously.
/// Returns 0 if write succeeds and -1 if write fails.
int SPIWrite(const uint8_t *Tx, size_t Length);
/// SPI data transfer.
/// Returns 0 if transfer succeeds and -1 if transfer fails.
int SPITransfer(const uint8_t *Tx, uint8_t *Rx, size_t Length);

/// @}

/// @defgroup GPIO GPIO
/// PinNum can be any pin in #ModulePinDef.
/// @{

/// GPIO level
typedef enum { GPIO_LOW = 0, GPIO_HIGH } GPIOLevel;
/// GPIO internal pull up/down.
typedef enum {
  GPIO_NO_PULL = 10,  ///< no module internal pull up or pull down
  GPIO_PULL_UP,       ///< module internal pull up
  GPIO_PULL_DOWN      ///< module internal pull down
} GPIOPull;
/// Set a GPIO to input mode and the internal pull up/down.
/// Returns 0 if succeeded and -1 if failed.
int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull);
/// Set a GPIO to output mode.
/// Returns 0 if succeeded and -1 if failed.
int GPIOSetModeOutput(uint8_t PinNum);
/// Set a GPIO output high.
/// Returns 0 if succeeded and -1 if failed.
int GPIOSetHigh(uint8_t PinNum);
/// Set a GPIO output low.
/// Returns 0 if succeeded and -1 if failed.
int GPIOSetLow(uint8_t PinNum);
/// Get input level of a GPIO.
/// Returns GPIOLevel if succeeded and -1 if failed.
int GPIOGet(uint8_t PinNum);
/// Set level polarity of a GPIO pin with wakeup capability for next wakeup.
/// Make sure to set the GPIO to input mode.
/// Returns 0 if succeeded and -1 if failed.
int GPIOSetWakeupLevel(uint8_t PinNum, GPIOLevel Level);
/// Disable wakeup capability of a GPIO pin.
int GPIODisableWakeup(uint8_t PinNum);

/// @}

/// @defgroup UART UART
/// @{

/// UART interfaces
typedef enum {
  UART_0 = 0,  ///< can be used for firmware update via the bootloader
  UART_1,      ///< used by GNSS
  LEUART       ///< low energy, only supports baudrate up to 9600
} UARTInterface;
/// Initialise a uart interface, options are not supported and should be 0.
/// UARTNum is the UART number defined by enum #UARTInterface.
/// Returns the handle if succeeded and NULL if failed.
void *UARTInit(UARTInterface UARTNum, uint32_t BaudRate, uint32_t Options);
/// Deintialise a UART interface.
void UARTDeinit(void *Handle);
/// Write to a UART interface synchronously.
/// Returns 0 if write succeeds and -1 if read fails.
int UARTWrite(void *Handle, const uint8_t *Tx, size_t Length);
/// Read from input buffer of a UART interface. The buffer size is 50 bytes.
/// Returns number of bytes read back and -1 if read fails.
int UARTRead(void *Handle, uint8_t *Rx, size_t Length);

/// @}

/// @defgroup Pulse_counter Pulse counter
/// @{

/// Pulse counter bit-wise options, can be ORed
enum PulseCounterOption {
  PCNT_DEFAULT_OPTIONS = 0,
  PCNT_EDGE_FALLING = 1 << 0,  ///< count on falling edge, default rising edge
  PCNT_DEBOUNCE_DISABLE =
      1 << 1,  ///< disable hardware debouncing, default enabled for about 160us
  PCNT_PULL_UP = 1 << 2  ///< enable pull-up, default pull-down
};
/// Initialise the pulse counter and configure the event generation logic.
/// Event is generated when pulse count hits multiple of Limit. Limit can be set
/// to 0 to 256, or multiple of 256. Set Limit to 0 to disable event generation.
/// Options are used to configure the pulse counter. Set option to 0 to
/// count on rising edge, and enable debouncing.
/// Returns 0 if succeeded and -1 if failed.
int PulseCounterInit(uint32_t Limit, uint32_t Options);
/// Get the total count of the pulse counter.
/// Returns the total count.
uint64_t PulseCounterGet(void);
/// Deinitialise the pulse counter.
void PulseCounterDeinit(void);

/// @}

/// @defgroup RF_Test Satellite radio RF test
/// @{

/// RF test Tx type
enum RFTestTxType {
  TX_TYPE_TONE = 0,  ///< transmit tone, i.e. CW
  TX_TYPE_PRBS       ///< transmit pseudorandom binary sequence
};
/// Start the RF Tx test. The default LED is on when transmitting.
/// The antenna port should be connected to a load when testing.
/// \p Frequency is in Hertz.
/// \p TxType can be tone or pseudorandom binary sequence defined by
/// #RFTestTxType. When testing in burst mode, the radio is on for 260ms and off
/// for 1740ms. Otherwise the radio transmits continuously. Returns 0 if
/// succeeded and -1 if failed.
int RFTestTxStart(uint32_t Frequency, uint8_t TxType, bool IsBurst);
/// Stop the RF Tx test.
void RFTestTxStop(void);

/// Turn on the satellite radio receiver.
/// \p Frequency is in Hertz.
/// Returns 0 if succeeded and -1 if failed.
int RFTestRxStart(uint32_t Frequency);
/// Get current RSSI from radio receiver.
/// \p RSSI is in dBm.
/// Returns 0 if succeeded and -1 if failed.
int RFTestRxRSSI(int32_t *RSSI);
/// Stop the RF Rx test.
void RFTestRxStop(void);

/// @}

/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_HARDWARE_API_H
