// Copyright (c) 2016-2026, Myriota Pty Ltd, All Rights Reserved
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
/// \brief Hardware interface related API.
/// \details For more details, see the
/// <a href="https://www2.myriota.com/module-datasheet">Official Datasheet</a>.
/// @{

/// @defgroup Module_pins Module pins
/// \brief Module I/O pin definitions.
/// @{

/// \brief Module pin definitions.
/// \details For more details, see the
/// <a href="https://www2.myriota.com/module-datasheet">Official Datasheet</a>.
enum ModulePinDef {
  PIN_BAND = 2,  ///< BAND - Read-only, do not connect. High on M1
  /// variant and low on M2 variant.
  PIN_ADC1 = 6,        ///< ADC1 - ADC port 1. GPIO.
  PIN_ADC0 = 8,        ///< ADC0 - ADC port 0. GPIO.
  PIN_PULSE1 = 10,     ///< PULSE1 - Pulse counter. GPIO. ADC.
  PIN_PULSE0 = 11,     ///< PULSE0 - Pulse counter. GPIO. ADC.
  PIN_UART0_RTS = 12,  ///< UART0_RTS - UART0 RTS, unused. GPIO.
  PIN_GPIO2 = 14,      ///< GPIO2 - GPIO.
  PIN_GPIO3 = 15,      ///< GPIO3 - GPIO.
  PIN_RF_EN = 17,      ///< RF_EN  - Output only. High when radio is enabled
  /// and low when radio is disabled.
  PIN_SPI_CS = 19,      ///< SPI_CS - SPI chip select. GPIO.
  PIN_SPI_SCK = 20,     ///< SPI_SCK - SPI SCLK. GPIO.
  PIN_SPI_MISO = 21,    ///< SPI_MISO - SPI MISO. GPIO.
  PIN_SPI_MOSI = 22,    ///< SPI_MOSI - SPI MOSI. GPIO.
  PIN_GPIO0_WKUP = 24,  ///< GPIO0_WKUP - GPIO. Can wake the Module from sleep
  /// mode.
  PIN_GPIO4 = 33,       ///< GPIO4 - GPIO.
  PIN_GPIO5 = 34,       ///< GPIO5 - GPIO.
  PIN_GPIO6 = 35,       ///< GPIO6 - GPIO.
  PIN_GPIO1_WKUP = 41,  ///< GPIO1_WKUP - GPIO. Can wake up the Module from
  /// sleep mode.
  PIN_UART1_RX = 48,  ///< UART1_RX - UART1 RX, input. GPIO.
  PIN_UART1_TX = 49,  ///< UART1_TX - UART1 TX, output. GPIO.
  PIN_I2C_SCL = 50,   ///< I2C_SCL - I2C SCL with built-in 2.2kOhm pull up.
  /// External pull up is not recommended.
  PIN_I2C_SDA = 51,  ///< I2C_SDA - I2C SDA with built-in 2.2kOhm pull up.
  /// External pull up is not recommended.
  PIN_GPIO8 = 52,      ///< GPIO8 - GPIO.
  PIN_GPIO7 = 53,      ///< GPIO7 - GPIO.
  PIN_UART0_CTS = 54,  ///< UART0_CTS - UART0 CTS, unused. GPIO.
  PIN_UART0_TX = 55,   ///< UART0_TX - UART0 TX, output. Bootloader TX.
  PIN_UART0_RX = 56,   ///< UART0_RX - UART0 RX, input. Bootloader RX.
  PIN_LEUART_RX = 57,  ///< LEUART_RX - LEUART RX. ADC.
  PIN_LEUART_TX = 58,  ///< LEUART_TX - LEUART TX. ADC.
  PIN_MAX              ///< Maximum pin count, not a valid pin number.
};

/// @}

/// @defgroup ADC ADC
/// \brief Analog-to-Digital Converter (ADC) interface for reading analog
/// signals.
/// @{

/// \brief Reference voltages.
typedef enum {
  ADC_REF_VIO = 0,  ///< Use VIO_REF as reference voltage.
  ADC_REF_2V5,      ///< Use 2.5V as reference voltage.
  ADC_REF_1V25      ///< Use 1.25V as reference voltage.
} ADCReference;
/// \brief Get the converted voltage (mV) from the MCU ADC port.
/// \param PinNum The ADC pin number. Can be PIN_ADC0 or PIN_ADC1.
/// \param Reference The reference voltage.
/// \param mV[out] Pointer to store the converted voltage in millivolts.
/// \return 0 if read succeeds, -1 if read fails.
int ADCGetVoltage(uint8_t PinNum, ADCReference Reference, uint32_t *mV);
/// \brief Get raw value from the ADC port (0 - 0xFFF).
/// \param PinNum The ADC pin number. Can be PIN_ADC0 or PIN_ADC1.
/// \param Reference The reference voltage.
/// \param Value[out] Pointer to store the raw ADC value.
/// \return 0 if read succeeds, -1 if read fails.
int ADCGetValue(uint8_t PinNum, ADCReference Reference, uint16_t *Value);

/// @}

/// @defgroup I2C_master I2C master
/// \brief Inter-Integrated Circuit (I2C) interface for communication with
/// sensors and peripherals.
/// @{

/// \brief I2C bit-wise options.
enum I2COption {
  I2C_DEFAULT_OPTIONS = 0,  ///< Up to ~100 kbit/s.
  I2C_FAST_MODE = 1 << 0,   ///< Up to ~400 kbit/s.
};
/// \brief Initialise I2C bus.
/// \return 0 if initialisation succeeds, -1 if initialisation fails.
int I2CInit(void);
/// \brief Initialise I2C bus with options.
/// \param Option I2C options for bus speed.
/// \return 0 if initialisation succeeds, -1 if initialisation fails.
int I2CInitEx(uint32_t Option);
/// \brief Deinitialise I2C bus.
/// \details Deinitialises the I2C bus and releases associated hardware
/// resources.
void I2CDeinit(void);
/// \brief Write to an I2C device synchronously.
/// \param DeviceAddress The I2C device address.
/// \param Command[in] Pointer to the command data.
/// \param CommandLength Length of the command data.
/// \return 0 if write succeeds, -1 if write fails.
int I2CWrite(uint16_t DeviceAddress, const uint8_t *Command,
             size_t CommandLength);
/// \brief Read from an I2C device synchronously.
/// \param DeviceAddress The I2C device address.
/// \param Command[in] Pointer to the command data.
/// \param CommandLength Length of the command data.
/// \param Rx[out] Pointer to the buffer to store the received data.
/// \param RxLength Length of the buffer to store the received data.
/// \return 0 if read succeeds, -1 if read fails.
int I2CRead(uint16_t DeviceAddress, const uint8_t *Command,
            size_t CommandLength, uint8_t *Rx, size_t RxLength);

/// @}

/// @defgroup SPI_master SPI master
/// \brief Serial Peripheral Interface (SPI) master for high-speed data
/// transfer.
/// @{

/// \brief SPI baudrate
enum SPIBaudrate {
  SPI_BAUDRATE_DEFAULT = 1000000,  ///< 1 MHz.
  SPI_BAUDRATE_MAX = 24000000      ///< 24 MHz.
};
/// \brief Initialise SPI master.
/// \param BaudRate The SPI baudrate.
/// \return 0 if initialisation succeeds, -1 if initialisation fails.
int SPIInit(uint32_t BaudRate);
/// \brief Deinitialise SPI master.
/// \details Disables SPI communications and releases associated hardware
/// resources.
void SPIDeinit(void);
/// \brief Write to an SPI device synchronously.
/// \param Tx[in] Pointer to the data to be transmitted.
/// \param Length Length of the data to be transmitted.
/// \return 0 if write succeeds, -1 if write fails.
int SPIWrite(const uint8_t *Tx, size_t Length);
/// \brief SPI data transfer.
/// \param Tx[in] Pointer to the data to be transmitted.
/// \param Rx[out] Pointer to the buffer to store the received data.
/// \param Length Length of the data to be transferred.
/// \return 0 if transfer succeeds, -1 if transfer fails.
int SPITransfer(const uint8_t *Tx, uint8_t *Rx, size_t Length);

/// @}

/// @defgroup GPIO GPIO
/// \brief General Purpose Input/Output (GPIO) interface for digital signal
/// control.
/// @{

/// \brief GPIO level.
typedef enum {
  GPIO_LOW = 0,  ///< Low logic level.
  GPIO_HIGH      ///< High logic level.
} GPIOLevel;
/// \brief GPIO internal pull up/down.
typedef enum {
  GPIO_NO_PULL = 10,  ///< No module internal pull up or pull down.
  GPIO_PULL_UP,       ///< Module internal pull up.
  GPIO_PULL_DOWN      ///< Module internal pull down.
} GPIOPull;
/// \brief Set a GPIO to input mode and the internal pull up/down.
/// \param PinNum The GPIO pin number from #ModulePinDef.
/// \param Pull The internal pull up/down configuration from #GPIOPull.
/// \return 0 if succeeded, -1 if failed.
int GPIOSetModeInput(uint8_t PinNum, GPIOPull Pull);
/// \brief Set a GPIO to output mode.
/// \param PinNum The GPIO pin number from #ModulePinDef.
/// \return 0 if succeeded, -1 if failed.
int GPIOSetModeOutput(uint8_t PinNum);
/// \brief Set a GPIO output high.
/// \param PinNum The GPIO pin number from #ModulePinDef.
/// \return 0 if succeeded, -1 if failed.
int GPIOSetHigh(uint8_t PinNum);
/// \brief Set a GPIO output low.
/// \param PinNum The GPIO pin number from #ModulePinDef.
/// \return 0 if succeeded, -1 if failed.
int GPIOSetLow(uint8_t PinNum);
/// \brief Get input level of a GPIO.
/// \param PinNum The GPIO pin number from #ModulePinDef.
/// \return GPIOLevel if succeeded, -1 if failed.
int GPIOGet(uint8_t PinNum);
/// \brief Set level polarity of a GPIO pin with wakeup capability for next
/// wakeup.
/// \param PinNum The GPIO pin number from #ModulePinDef with wakeup capability.
/// \param Level The wakeup level from #GPIOLevel.
/// \return 0 if succeeded, -1 if failed.
int GPIOSetWakeupLevel(uint8_t PinNum, GPIOLevel Level);
/// \brief Disable wakeup capability of a GPIO pin.
/// \param PinNum The GPIO pin number from #ModulePinDef with wakeup capability.
/// \return 0 if succeeded, -1 if failed (e.g., pin does not support wakeup).
int GPIODisableWakeup(uint8_t PinNum);

/// @}

/// @defgroup UART UART
/// \brief Universal Asynchronous Receiver/Transmitter interfaces for serial
/// communication.
/// @{

/// \brief UART interfaces.
typedef enum {
  UART_0 = 0,  ///< Can be used for firmware update via the bootloader.
  UART_1,      ///< Used by GNSS in the Developer Kit.
  LEUART       ///< Low energy, only supports baudrate up to 9600.
} UARTInterface;

/// \brief UART configuration bitflags.
/// \note UART_STOPBITS_HALF and UART_STOPBITS_ONEANDHALF are not supported for
/// the LEUART interface.
/// \note Parity flags and Stop Bit flags are mutually exclusive within their
/// respective categories.
enum UARTOption {
  UART_DEFAULT_OPTIONS = 0x00000000,  ///< Default setting is 8N1 (8 data bits,
                                      ///< no parity, 1 stop bit).
  UART_PARITY_ODD = 0x00000001,  ///< Enable odd parity generation and checking.
  UART_PARITY_EVEN =
      0x00000002,  ///< Enable even parity generation and checking.
  UART_DATABITS_NINE = 0x00000004,  ///< Configure payload for 9 data bits
                                    ///< instead of the default 8.
  UART_STOPBITS_HALF = 0x00000008,  ///< Configure frame to use 0.5 stop bits.
  UART_STOPBITS_ONEANDHALF =
      0x00000010,                  ///< Configure frame to use 1.5 stop bits.
  UART_STOPBITS_TWO = 0x00000020,  ///< Configure frame to use 2 stop bits.
};

/// \brief Initialise a UART interface.
/// \param UARTNum The UART number defined by enum #UARTInterface.
/// \param BaudRate The baud rate for the UART communication.
/// \param Options Options for UART configuration. Bitmask of #UARTOption flags.
/// \return The handle if succeeded and NULL if failed.
void *UARTInit(UARTInterface UARTNum, uint32_t BaudRate, uint32_t Options);
/// \brief Deinitialise a UART interface.
/// \param Handle[in] The UART handle returned by UARTInit().
void UARTDeinit(void *Handle);
/// \brief Write to a UART interface synchronously.
/// \param Handle[in] The UART handle returned by UARTInit().
/// \param Tx[in] Pointer to the data to be transmitted.
/// \param Length Number of bytes to transmit.
/// \return 0 if write succeeds and -1 if write fails.
int UARTWrite(void *Handle, const uint8_t *Tx, size_t Length);
/// \brief Read from a UART interface synchronously.
/// \param Handle[in] The UART handle returned by UARTInit().
/// \param Rx[out] Pointer to the buffer to store received data.
/// \param Length Number of bytes to read.
/// \return Number of bytes read if successful, -1 if read fails.
int UARTRead(void *Handle, uint8_t *Rx, size_t Length);

/// @}

/// @defgroup Pulse_counter Pulse counter
/// \brief Pulse Counter for counting pulses on an input while the Module is in
/// sleep mode.
/// @{

/// \brief Pulse counter bit-wise options.
enum PulseCounterOption {
  PCNT_DEFAULT_OPTIONS = 0,        ///< Count on rising edge.
  PCNT_EDGE_FALLING = 1 << 0,      ///< Count on falling edge.
  PCNT_DEBOUNCE_DISABLE = 1 << 1,  ///< Disable hardware debouncing, default
  /// enabled for about 160us.
  PCNT_PULL_UP = 1 << 2  ///< Enable pull-up, default pull-down.
};
/// \brief Initialise the pulse counter and configure the event generation
/// logic.
/// \details Event is generated when pulse count hits multiple of Limit.
/// \param Limit The pulse count limit for event generation. Can be set to 0 to
/// 256, or multiple of 256. Set Limit to 0 to disable event generation.
/// \param Options Used to configure the pulse counter. See #PulseCounterOption
/// for available options.
/// \return 0 if succeeded, -1 if failed.
int PulseCounterInit(uint32_t Limit, uint32_t Options);
/// \brief Get the total count of the pulse counter.
/// \return The total count accumulated since PulseCounterInit() was called.
uint64_t PulseCounterGet(void);
/// \brief Deinitialise the pulse counter.
void PulseCounterDeinit(void);

/// @}

/// @defgroup RF_Test Satellite radio RF test
/// \brief Satellite radio RF testing and diagnostic functions.
/// @{

/// \brief RF test Tx type.
enum RFTestTxType {
  TX_TYPE_TONE = 0,  ///< Transmit tone, i.e. CW.
  TX_TYPE_PRBS       ///< Transmit pseudorandom binary sequence.
};
/// \brief Start the RF Tx test.
/// \details The default LED is on when transmitting. The antenna port should
/// be connected to a load when testing. When testing in burst mode, the radio
/// is on for 260ms and off for 1740ms. Otherwise the radio transmits
/// continuously.
/// \param Frequency The frequency in Hertz.
/// \param TxType The type of transmission, either tone or pseudorandom binary
/// sequence defined by #RFTestTxType.
/// \param IsBurst Whether to transmit in burst mode.
/// \return 0 if succeeded, -1 if failed.
int RFTestTxStart(uint32_t Frequency, uint8_t TxType, bool IsBurst);
/// \brief Stop the RF Tx test.
/// \details Terminates RF transmission and returns the radio to idle state.
/// \see RFTestTxStart()
void RFTestTxStop(void);

/// \brief Start the RF Rx test.
/// \param Frequency The frequency in Hertz.
/// \return 0 if succeeded, -1 if failed.
int RFTestRxStart(uint32_t Frequency);
/// \brief Get current RSSI from radio receiver.
/// \param RSSI[out] Pointer to an integer to store the RSSI value in dBm.
/// \return 0 if succeeded, -1 if failed.
int RFTestRxRSSI(int32_t *RSSI);
/// \brief Stop the RF Rx test.
/// \details Terminates RF reception and returns the radio to idle state.
/// \see RFTestRxStart()
void RFTestRxStop(void);

/// @}

/// @defgroup RF_Stats Satellite radio statistics
/// \brief Receiver statistics for downlink message tracking.
/// @{

/// \brief Rx statistics type.
typedef struct {
  uint16_t attempts;   ///< Number of attempts made to receive a packet.
  uint16_t successes;  ///< Number of successful packets.
} RxStats_t;

/// \brief Get satellite radio receiver (downlink) statistics.
/// \param Stats Pointer to an RxStats_t structure to populate with statistics.
void RxStatsGet(void *Stats);

/// @}

/// @defgroup User_NVRAM User NVRAM
/// \brief Non-volatile memory for persistent storage.
/// @{

/// \brief Size of the user NVRAM in bytes.
#define NVRAM_MEM_SIZE 2048
/// @cond
#define NVRAM_MEM __attribute__((section("user_nvram"))) volatile const
/// @endcond

/// \brief Clear the NVRAM.
/// \details NVRAM should be cleared before overwriting the existing contents.
/// The NVRAM is filled with 0xFF after being cleared.
void NvramClear();
/// \brief Write buffer contents to NVRAM, starting from offset.
/// \param Offset The offset in NVRAM to start writing.
/// \param Buf[in] Pointer to the buffer containing data to write.
/// \param Length The number of bytes to write.
/// \return 0 if succeeded, -1 if failed.
int NvramWrite(uint32_t Offset, const uint8_t *Buf, size_t Length);

/// @}
///
/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_HARDWARE_API_H
