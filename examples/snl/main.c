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

// A simple application running on Myriota's "Sense and Locate" board. Reads
// from 4-20mA sensor periodically and sends messages to satellite containing
// device location, time and sensor reading.

#include "myriota_user_api.h"

const static uint8_t VibrationGPIO = PIN_GPIO1_WKUP;

// Format of the messages to be transmitted. Values are little endian
typedef struct {
  uint16_t sequence_number;
  int32_t latitude;   // scaled by 1e7, e.g. -891234567 (south 89.1234567)
  int32_t longitude;  // scaled by 1e7, e.g. 1791234567 (east 179.1234567)
  uint32_t time;      // epoch timestamp of the reading
  uint32_t mV;        // sensor reading in millivolt
} __attribute__((packed)) sensor_message;

static sensor_message message = {0};

// Default values
enum {
  // GPIO to enable/disable 4-20mA circuit
  GPIO_4_20_ENABLE = PIN_PULSE0,
  // ADC instance sensing 4-20mA circuit
  ADC_PRESSURE_SENSOR = PIN_ADC0,
  // Delay to allow 21V power supply to stablise
  DELAY_MS_21V_STABILISE = 700,
  // Message sending period
  MESSAGE_PERIOD_HOURS = 8
};

static int read_sensor(uint32_t *value) {
  int result;

  GPIOSetHigh(GPIO_4_20_ENABLE);
  Delay(DELAY_MS_21V_STABILISE);

  // Depending on the measurement input range, different ADC reference voltage
  // can be used
  result = ADCGetVoltage(PIN_ADC0, ADC_REF_1V25, value);
  if (result != 0) printf("Error reading sensor: %i", result);

  // Disable 21V supply
  GPIOSetLow(GPIO_4_20_ENABLE);
  return result;
}

// Display values
static uint32_t Measure_4_20() {
  uint32_t mv;
  if (read_sensor(&mv) != 0)
    printf("Failed to get ADC voltage\n");
  else
    printf("Voltage = %umV\n", (unsigned)mv);
  return mv;
}

static time_t SendMessage(void) {
  static uint16_t sequence_number = 0;

  message.sequence_number = sequence_number;
  message.mV = Measure_4_20();
  message.time = TimeGet();
  if (GNSSFix()) printf("Failed to get GNSS Fix, using last known fix\n");
  LocationGet(&(message.latitude), &(message.longitude), NULL);

  ScheduleMessage((void *)&message, sizeof(message));

  printf("Scheduled message: %u %f %f %u %u\n", sequence_number,
         (message.latitude) * 1e-7, (message.longitude) * 1e-7,
         (unsigned int)(message.time), (unsigned)(message.mV));
  sequence_number++;

  return HoursFromNow(MESSAGE_PERIOD_HOURS);
}

// Vibration sensor
static time_t RunsOnGPIOWakeup() {
  printf("Woken up by vibration sensor at %u\n", (unsigned int)TimeGet());
  return OnGPIOWakeup();
}

void AppInit() {
  // Vibation sensor init
  // Internally pull down the pin to make sure the pin is not floating
  GPIOSetModeInput(VibrationGPIO, GPIO_NO_PULL);
  ScheduleJob(RunsOnGPIOWakeup, OnGPIOWakeup());
  GPIOSetWakeupLevel(VibrationGPIO, GPIO_LOW);

  // Schedule message sending job
  ScheduleJob(SendMessage, ASAP());
}

int BoardStart(void) {
  // Make sure 4-20mA circuit is disabled by default
  GPIOSetModeOutput(GPIO_4_20_ENABLE);
  GPIOSetLow(GPIO_4_20_ENABLE);

  // Blink once after application starts
  LedTurnOn();
  Delay(200);
  LedTurnOff();

  printf("Myriota sense and locate example\n");

  return 0;
}
