// Copyright (c) 2021, Myriota Pty Ltd, All Rights Reserved
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

// This example demonstrates how to use receive related RF test API.
// Further information can be found in the README file
//
// Like the blinky example, this example runs in sandbox mode.

#include <string.h>
#include <unistd.h>
#include "myriota_user_api.h"

#define GPIO_QUERY_PIN PIN_GPIO0_WKUP
#define RF_FREQUENCY_Hz 400550000
#define LOG_CODE_RX_STATS 1

#if defined(LAB_TEST)
#define POLL_INTERVAL 10  // Poll every 10 seconds
#define LOG_TO_FLASH 0
#else
#define POLL_INTERVAL 7200  // Poll every 2 hours
#define LOG_TO_FLASH 1
#endif

static RxStats_t prev_stats = {0, 0, 0};

static void LedBlink(uint32_t DelayMs, uint32_t RepeatTime) {
  while (RepeatTime--) {
    LedTurnOn();
    Delay(DelayMs);
    LedTurnOff();
    // No need to delay for the last blink
    if (RepeatTime) Delay(DelayMs);
  }
}

static void QueryResult() {
  RxStats_t stats = {0, 0, 0};

  RxStatsGet(&stats);

  // Using LED to indicates receive status:
  // Received at least one verified packet: blink 3 times
  // Received packets but all unverified  : blink 2 times
  // Received no packet at all            : blink 1 once
  if (stats.verified > prev_stats.verified)
    LedBlink(250, 3);
  else if (stats.unverified > prev_stats.unverified)
    LedBlink(250, 2);
  else
    LedBlink(250, 1);
  prev_stats = stats;

#if (LOG_TO_FLASH == 1)
  LogAdd(LOG_CODE_RX_STATS, &stats, sizeof(RxStats_t));
#endif

  printf("\n%lu rx packets: Attempts %-6u Unverified %-6u Verified %-6u\n",
         (uint32_t)time(NULL), stats.attempts, stats.unverified,
         stats.verified);
}

static time_t GetRxPacketStats() {
  QueryResult();

  return SecondsFromNow(POLL_INTERVAL);
}

static time_t GpioWakeupHandler() {
  // Always retrieve receiver stats before any action.
  QueryResult();
  memset(&prev_stats, 0, sizeof(prev_stats));

  // Stop test and suspend the module
  ScheduleJob(GetRxPacketStats, Never());
  SuspendModeEnable(true);

  Delay(1000);

  SuspendModeEnable(false);
  ScheduleJob(GetRxPacketStats, SecondsFromNow(5));
  printf("Packet verify test is restarted\n");

  return OnGPIOWakeup();
}

void AppInit() {
  GPIOSetModeInput(GPIO_QUERY_PIN, GPIO_PULL_DOWN);
  GPIOSetWakeupLevel(GPIO_QUERY_PIN, GPIO_HIGH);

  // Ensure the module is not in suspend mode
  if (SuspendModeIsEnabled()) SuspendModeEnable(false);

  ScheduleJob(GetRxPacketStats, SecondsFromNow(5));
  ScheduleJob(GpioWakeupHandler, OnGPIOWakeup());
}
