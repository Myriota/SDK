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

// A demo application running on Myriota's "Sense and Locate" board. Reads from
// 4-20mA sensor periodically and sends messages to satellite containing the
// device location, timestamp and current in uA. The application handles wakeup
// button and vibration sensor events as well.

#include "myriota_user_api.h"

#define SHUNT_RESISTANCE 100  // ohm
#define LED_DELAY 200         // ms

const static uint8_t ButtonGPIO = PIN_GPIO0_WKUP;
const static uint8_t VibrationGPIO = PIN_GPIO1_WKUP;

#define VIBRATION_SENSOR_ENABLED false  // true to enable vibration sensor

// Format of the messages to be transmitted. Values are little endian
typedef struct {
  uint16_t sequence_number;
  int32_t latitude;   // scaled by 1e7, e.g. -891234567 (south 89.1234567)
  int32_t longitude;  // scaled by 1e7, e.g. 1791234567 (east 179.1234567)
  uint32_t time;      // epoch timestamp of the reading
  uint32_t current;   // sensor current in uA
} __attribute__((packed)) sensor_message;

// Default values
enum {
  // GPIO to enable/disable 4-20mA circuit
  GPIO_4_20_ENABLE = PIN_PULSE0,
  // ADC instance sensing 4-20mA circuit
  ADC_PRESSURE_SENSOR = PIN_ADC0,
  // Modify this delay to save power based on sensor stabilisation time
  DELAY_MS_21V_STABILISE = 1500,
  // Message sending period
  MESSAGE_PERIOD_HOURS = 8
};

static int ReadSensor(uint32_t *value) {
  int result;

  GPIOSetHigh(GPIO_4_20_ENABLE);
  Delay(DELAY_MS_21V_STABILISE);

  result = ADCGetVoltage(PIN_ADC0, ADC_REF_2V5, value);
  if (result != 0) printf("Error reading sensor: %i", result);

  GPIOSetLow(GPIO_4_20_ENABLE);  // Disable 21V supply

  return result;
}

static uint32_t MeasureCurrent() {
  uint32_t mv, ua = 0;

  if (ReadSensor(&mv) != 0)
    printf("Failed to get ADC voltage\n");
  else {
    ua = mv * 1000 / SHUNT_RESISTANCE;
    printf("Current = %uuA\n", (unsigned)ua);
  }

  return ua;
}

static time_t SendMessage(void) {
  static uint16_t sequence_number = 0;
  int32_t lat, lon;
  time_t timestamp;
  uint32_t current;

  if (GNSSFix()) printf("Failed to get GNSS Fix, using last known fix\n");
  LocationGet(&lat, &lon, &timestamp);

  current = MeasureCurrent();

  const sensor_message message = {sequence_number, lat, lon, timestamp,
                                  current};
  ScheduleMessage((void *)&message, sizeof(message));

  printf("Scheduled message: %u %f %f %u %u\n", sequence_number, lat * 1e-7,
         lon * 1e-7, (unsigned int)(timestamp), (unsigned)current);

  sequence_number++;

  return HoursFromNow(MESSAGE_PERIOD_HOURS);
}

static time_t RunsOnGPIOWakeup() {
  if (GPIOGet(ButtonGPIO) == GPIO_HIGH) {
    printf("Woken up by button at %u\n", (unsigned int)TimeGet());
  }

  if (VIBRATION_SENSOR_ENABLED && GPIOGet(VibrationGPIO) == GPIO_LOW) {
    printf("Woken up by vibration sensor at %u\n", (unsigned int)TimeGet());
  }

  return OnGPIOWakeup();
}

void AppInit() {
  if (VIBRATION_SENSOR_ENABLED) {
    GPIOSetModeInput(VibrationGPIO, GPIO_NO_PULL);
    GPIOSetWakeupLevel(VibrationGPIO, GPIO_LOW);
  }

  GPIOSetModeInput(ButtonGPIO, GPIO_PULL_DOWN);
  GPIOSetWakeupLevel(ButtonGPIO, GPIO_HIGH);

  ScheduleJob(RunsOnGPIOWakeup, OnGPIOWakeup());
  ScheduleJob(SendMessage, ASAP());
}

int BoardStart(void) {
  // Make sure 4-20mA circuit is disabled by default
  GPIOSetModeOutput(GPIO_4_20_ENABLE);
  GPIOSetLow(GPIO_4_20_ENABLE);

  LedTurnOn();
  Delay(LED_DELAY);
  LedTurnOff();

  printf("Myriota sense and locate example\n");

  return 0;
}
