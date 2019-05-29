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

#include "bsp.h"
#include "myriota_hardware_api.h"
#include "myriota_user_api.h"

#define LED_PIN PIN_GPIO3
#define GNSS_EN_PIN PIN_GPIO4
// Backward support of development board revision 1
#define ANT_SEL_PIN_REV1 PIN_RF_TEST2
#define ANT_SEL_PIN PIN_GPIO6
#define DEBUG_INTERFACE UART_0
#define DEBUG_BAUDRATE 115200
#define MODULE_BAND_PIN PIN_BAND

static void *DebugHandle;

#ifdef LAB_TEST
#define BOARD_ENV "GNSSFIX=0;DUMPTX=1"
#else
#define BOARD_ENV ""
#endif

__attribute__((weak)) char *BoardEnvGet() {
  BUILD_BUG_ON(sizeof(BOARD_ENV) > BOARD_ENV_LEN_MAX);
  return BOARD_ENV;
}

int BoardInit(void) {
  BoardLEDDeinit();
  // Avoid current leak from the battery measurement switch when floating
  GPIOSetModeInput(PIN_GPIO2, GPIO_PULL_DOWN);
  return 0;
}

__attribute__((weak)) int BoardStart(void) {
  LedTurnOn();
  Delay(200);
  LedTurnOff();
  GPIOSetModeInput(MODULE_BAND_PIN, GPIO_NO_PULL);
  printf("Myriota development board %s variant\n",
         GPIOGet(MODULE_BAND_PIN) == GPIO_HIGH ? "VHF" : "UHF");

  return 0;
}

int BoardBatteryVoltGet(uint32_t *mv) {
// Compensate for the voltage drop across the diode Q202
#define VEXT_COMPENSATE 15    // [mV]
#define SWITCH_SETTLE_TIME 1  // [ms]
#define AVERAGE_COUNT 3

  const uint8_t ADCPin = PIN_ADC1;
  const uint8_t ControlPin =
      PIN_GPIO2;  // Pin to control the switch for the measurement

  // Check development board revision
  GPIOSetModeInput(PIN_ADC1, GPIO_NO_PULL);
  bool IsRev1 = (GPIOGet(PIN_ADC1) == GPIO_HIGH);

  GPIOSetModeOutput(ControlPin);
  GPIOSetHigh(ControlPin);
  Delay(SWITCH_SETTLE_TIME);

  uint32_t batt = 0, volt = 0;
  for (unsigned i = 0; i < AVERAGE_COUNT; i++) {
    ADCReference Ref;
    if (IsRev1)
      Ref = ADC_REF_VIO;  // May clip if battery voltage is higher than VIO
    else
      Ref = ADC_REF_2V5;  // Battery voltage won't be higher than 5V
    if (ADCGetVoltage(ADCPin, Ref, &batt)) {
      printf("Failed to get battery voltage\n");
      GPIOSetModeInput(ControlPin, GPIO_PULL_DOWN);
      return -1;
    } else {
      volt += batt + VEXT_COMPENSATE;
    }
  }
  GPIOSetModeInput(ControlPin, GPIO_PULL_DOWN);
  *mv = volt / AVERAGE_COUNT;
  if (!IsRev1) *mv *= 2;  // Dividor on the development board
  return 0;
}

void BoardLEDInit(void) {
  GPIOSetModeOutput(PIN_GPIO3);
  GPIOSetLow(PIN_GPIO3);
}

void BoardLEDDeinit(void) { GPIOSetModeInput(LED_PIN, GPIO_PULL_DOWN); }

void BoardLEDTurnOn(void) { GPIOSetHigh(LED_PIN); }

void BoardLEDTurnOff(void) { GPIOSetLow(LED_PIN); }

void BoardLEDToggle(void) {
  if (GPIO_LOW == GPIOGet(LED_PIN))
    BoardLEDTurnOn();
  else
    BoardLEDTurnOff();
}

void BoardGNSSPowerEnable(void) {
  GPIOSetModeOutput(GNSS_EN_PIN);
  GPIOSetHigh(GNSS_EN_PIN);
}

void BoardGNSSPowerDisable(void) {
  GPIOSetModeOutput(GNSS_EN_PIN);
  GPIOSetLow(GNSS_EN_PIN);
}

bool BoardGNSSPowerIsEnabled(void) { return GPIOGet(GNSS_EN_PIN); }

__attribute__((weak)) int BoardAntennaSelect(bool IsTx, RadioBand Band) {
  (void)(IsTx);

  bool OnBoardAntenna = true;
  if (Band == RADIO_BAND_VHF) OnBoardAntenna = false;

  // Support all development board revisions
  GPIOSetModeOutput(ANT_SEL_PIN_REV1);
  GPIOSetModeOutput(ANT_SEL_PIN);

  if (OnBoardAntenna) {
    GPIOSetHigh(ANT_SEL_PIN_REV1);
    GPIOSetHigh(ANT_SEL_PIN);
  } else {
    GPIOSetLow(ANT_SEL_PIN_REV1);
    GPIOSetLow(ANT_SEL_PIN);
  }

  return 0;
}

void *BoardDebugInit(void) {
  DebugHandle = UARTInit(DEBUG_INTERFACE, DEBUG_BAUDRATE, 0);
  return DebugHandle;
}

void BoardDebugDeinit(void) { UARTDeinit(DebugHandle); }

int BoardDebugWrite(const uint8_t *Tx, size_t Length) {
  return UARTWrite(DebugHandle, Tx, Length);
}
