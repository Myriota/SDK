// Copyright (c) 2016-2022, Myriota Pty Ltd, All Rights Reserved
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

// A demo application running on Myriota's "Sense and Locate" board.
// Reads from 4 - 20mA sensor periodically and sends messages to satellite
// containing the device location, timestamp, current in uA and battery voltage
// in mV. The application handles wakeup button and vibration sensor events as
// well.

#include "myriota_user_api.h"

#define VIBRATION_SENSOR_ENABLED false  // true to enable vibration sensor

#define LED_DELAY 200              // ms
#define SHUNT_RESISTANCE 100       // ohm
#define SENSOR_TOLERANCE 0.1f      // +-10%
#define SENSOR_TEST_INTERVAL 5000  // ms

const static uint8_t ButtonGPIO = PIN_GPIO0_WKUP;
const static uint8_t VibrationGPIO = PIN_GPIO1_WKUP;

// Format of the messages to be transmitted. Values are little endian
typedef struct {
  uint16_t sequence_number;
  int32_t latitude;   // scaled by 1e7, e.g. -891234567 (south 89.1234567)
  int32_t longitude;  // scaled by 1e7, e.g. 1791234567 (east 179.1234567)
  uint32_t time;      // epoch timestamp of the reading
  uint32_t current;   // sensor current in uA
  uint16_t battery_voltage;  // battery voltage in mV
} __attribute__((packed)) sensor_message;

// Default values
enum {
  // GPIO to enable/disable 4-20mA circuit
  GPIO_4_20_ENABLE = PIN_PULSE0,
  // ADC instance sensing 4-20mA circuit
  ADC_PRESSURE_SENSOR = PIN_ADC0,
  // Modify this delay to save power based on sensor stabilisation time
  DELAY_MS_21V_STABILISE = 1500,
  // Message per day
  MESSAGE_PER_DAY = 3
};

static void LedBlink(uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    LedTurnOn();
    Delay(LED_DELAY);
    LedTurnOff();
    if (i < count - 1) Delay(LED_DELAY);
  }
}

static int ReadSensor(uint32_t *value) {
  GPIOSetHigh(GPIO_4_20_ENABLE);
  Delay(DELAY_MS_21V_STABILISE);

  int result = ADCGetVoltage(PIN_ADC0, ADC_REF_2V5, value);
  if (result != 0) printf("Error reading sensor: %i", result);

  GPIOSetLow(GPIO_4_20_ENABLE);

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

static void DisplaySensorResult(uint32_t current) {
  if (current < 4000 * (1 - SENSOR_TOLERANCE) ||
      current > 20000 * (1 + SENSOR_TOLERANCE)) {
    if (current < 200) {
      printf("Sensor disconnected\n");
      LedBlink(3);
    } else {
      printf("Sensor reading out of range\n");
      LedBlink(2);
    }
  } else {
    printf("Sensor reading OK\n");
    LedBlink(1);
  }
}

static time_t SendMessage(void) {
  static uint16_t sequence_number = 0;
  int32_t lat, lon;
  time_t next_schedule;
  uint32_t timestamp, current, volt_32;

  next_schedule = TimeGet() + 24 * 3600 / MESSAGE_PER_DAY;

  if (GNSSFix()) printf("Failed to get GNSS Fix, using last known fix\n");
  LocationGet(&lat, &lon, NULL);

  timestamp = TimeGet();
  current = MeasureCurrent();
  BatteryGetVoltage(&volt_32);
  uint16_t voltage = (uint16_t)volt_32;

  const sensor_message message = {sequence_number, lat,     lon,
                                  timestamp,       current, voltage};
  ScheduleMessage((void *)&message, sizeof(message));

  printf("Scheduled message: %u %f %f %u %u %u\n", sequence_number, lat * 1e-7,
         lon * 1e-7, (unsigned int)timestamp, (unsigned int)current, voltage);

  sequence_number++;

  return next_schedule;
}

static time_t RunsOnGPIOWakeup() {
  if (GPIOGet(ButtonGPIO) == GPIO_HIGH) {
    DisplaySensorResult(MeasureCurrent());
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

  ScheduleJob(RunsOnGPIOWakeup, OnGPIOWakeup());
  ScheduleJob(SendMessage, ASAP());
}

int BoardStart(void) {
  GPIOSetModeOutput(GPIO_4_20_ENABLE);
  GPIOSetLow(GPIO_4_20_ENABLE);

  GPIOSetModeInput(ButtonGPIO, GPIO_PULL_DOWN);
  GPIOSetWakeupLevel(ButtonGPIO, GPIO_HIGH);

  printf("Myriota sense and locate example, %d messages per day\n",
         MESSAGE_PER_DAY);
  printf("Module ID: %s\n", ModuleIDGet());
  printf("Registration code: %s\n", RegistrationCodeGet());

  if (GPIOGet(ButtonGPIO) == GPIO_HIGH) {
    bool button_hold = true;
    for (uint8_t i = 0; i < 10; i++) {
      Delay(100);
      if (GPIOGet(ButtonGPIO) == GPIO_LOW) {
        button_hold = false;
        break;
      }
    }
    if (button_hold) {
      printf("Sensor test mode, reset to exit...\n");
      while (1) {
        DisplaySensorResult(MeasureCurrent());
        Delay(SENSOR_TEST_INTERVAL);
      }
    }
  } else {
    DisplaySensorResult(MeasureCurrent());
  }

  return 0;
}
