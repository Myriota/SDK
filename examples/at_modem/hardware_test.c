// Copyright (c) 2021-2025, Myriota Pty Ltd, All Rights Reserved
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

#include <stdarg.h>
#include <unistd.h>
#include "at.h"

#define TEST_COMMAND_LEN 50

#define ANT_SEL_PIN PIN_GPIO6
#define GNSS_EN_PIN PIN_GPIO4

// Test time in seconds
#define GNSS_TEST_TIME 10
#define RF_TX_TEST_TIME 10
#define RF_RX_TEST_TIME 10

#define GNSS_COMM_TEST_ONLY false  // false for full test
#define GNSS_SENTENCE_PASS_COUNT 50
#define GNSS_DEFAULT_BAUDRATE 9600
#define NMEA_BUF_LENGTH (79 + 1)
#define NMEA_CHECKSUM_LEN 2
#define NMEA_START_CHAR '$'
#define NMEA_END_CHAR '\r'
#define NMEA_CHECKSUM_DEL '*'

static void *GNSSHandle = NULL;
static char NMEASentence[NMEA_BUF_LENGTH];

bool IsTestMode(const uint32_t Timeout) {
  char rx[TEST_COMMAND_LEN + 1] = {0};
  uint32_t start_tick = TickGet();
  int total_len = 0;
  while (TickGet() <= start_tick + Timeout && total_len < TEST_COMMAND_LEN) {
    total_len += ATReceiveTimeout(rx + total_len, TEST_COMMAND_LEN - total_len);
    if (strstr(rx, MODEM_TEST_STR) != NULL) {
      return true;
    }
  }
  return false;
}

static void PRINT(const char *format, ...) {
  char dest[256];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(dest, format, argptr);
  va_end(argptr);
  printf("%s", dest);
  ATSend(dest);
}

static void GNSSInit() {
  GPIOSetModeOutput(GNSS_EN_PIN);
  GPIOSetHigh(GNSS_EN_PIN);
  GNSSHandle = UARTInit(UART_1, GNSS_DEFAULT_BAUDRATE, 0);
}

static void GNSSDeinit() {
  UARTDeinit(GNSSHandle);
  GNSSHandle = NULL;
  GPIOSetModeOutput(GNSS_EN_PIN);
  GPIOSetLow(GNSS_EN_PIN);
}

static bool NMEAChecksumCheck(const char *Sentence) {
  const char *checksum_start =
      Sentence + strlen(Sentence) - NMEA_CHECKSUM_LEN - 1;
  if (*checksum_start != NMEA_CHECKSUM_DEL) {
    PRINT("Checksum not found\n");
    return false;
  }
  uint16_t checksum = 0x00;
  for (char *Str = (char *)Sentence; Str < checksum_start; Str++) {
    checksum ^= *Str;
  }
  char checksum_str[NMEA_CHECKSUM_LEN * 2 + 1] = {0};
  snprintf(checksum_str, sizeof(checksum_str), "%02" PRIX16, checksum);
  // XXX assuming upper case in NMEA sentence
  if (strcmp(checksum_str, checksum_start + 1) == 0)
    return true;
  else {
    PRINT("Failed checksum check %s vs %s\n", checksum_str, checksum_start + 1);
    return false;
  }
}

static char *NMEAReadSentence(void) {
  uint8_t ch;
  uint32_t tick_start = TickGet();
  size_t index = 0;
  bool got_sentence = false;
  while (TickGet() - tick_start < TICK_PER_SECOND) {
    if (UARTRead(GNSSHandle, &ch, 1) != 1) {
      continue;
    }
    if (ch == NMEA_START_CHAR) {
      memset(NMEASentence, 0, sizeof(NMEASentence));
      index = 0;
      got_sentence = true;
      continue;
    }
    if (got_sentence && (ch == NMEA_END_CHAR)) {
      if (NMEAChecksumCheck(NMEASentence))
        return NMEASentence;
      else {
        index = 0;
        got_sentence = false;
      }
    }
    if (got_sentence) {
      NMEASentence[index++] = ch;
    }
    if (index == sizeof(NMEASentence)) {
      NMEASentence[sizeof(NMEASentence) - 1] = '\0';
      PRINT("Sentence too long %s\n", NMEASentence);
      got_sentence = false;
      index = 0;
    }
  }
  return NULL;
}

static bool GNSSTest(bool CommTestOnly) {
  bool test_result = true;
  PRINT("Testing GNSS...\n");
  if (CommTestOnly) {
    GNSSInit();
    uint32_t tick_start = TickGet();
    char *sentence;
    uint8_t sentence_count = 0;
    while (TickGet() - tick_start < GNSS_TEST_TIME * TICK_PER_SECOND) {
      if ((sentence = NMEAReadSentence()) != NULL) {
        PRINT("Found sentence: %s\n", sentence);
        sentence_count++;
      }
    }
    GNSSDeinit();
    if (sentence_count >= GNSS_SENTENCE_PASS_COUNT) {
      PRINT("GNSS test passed\n");
    } else {
      test_result = false;
      PRINT("GNSS test failed\n");
    }
  } else {
    // GNSS signal required, 90s timeout
    if (GNSSFix()) {
      test_result = false;
      PRINT("GNSS test failed\n");
    } else {
      int32_t lat, lon;
      time_t timestamp;
      LocationGet(&lat, &lon, &timestamp);
      PRINT("GNSS test passed: %f %f %u\n", lat * 1e-7, lon * 1e-7,
            (unsigned int)timestamp);
    }
  }
  PRINT("GNSS test stopped\n");
  return test_result;
}

static bool RFTXTest() {
  bool test_result = true;
  PRINT("Testing RF TX...\n");
  uint32_t tx_freq;
  if (GPIOGet(MODULE_BAND_PIN) == GPIO_HIGH) {
    tx_freq = VHF_TX_DEFAULT_FREQUENCY;
  } else {
    tx_freq = UHF_TX_DEFAULT_FREQUENCY;
  }
  if (RFTestTxStart(tx_freq, TX_TYPE_TONE, false)) {
    test_result = false;
    PRINT("RF TX test failed\n");
  } else {
    uint32_t tick_start = TickGet();
    while (TickGet() - tick_start < RF_TX_TEST_TIME * TICK_PER_SECOND);
  }
  RFTestTxStop();
  PRINT("RF TX test stopped\n");
  return test_result;
}

static bool RFRXTest() {
  bool test_result = true;
  PRINT("Testing RF RX...\n");
  RFTestRxStart(UHF_RX_DEFAULT_FREQUENCY);
  int32_t rssi;
  uint32_t tick_start = TickGet();
  while (TickGet() - tick_start < RF_RX_TEST_TIME * TICK_PER_SECOND) {
    if (RFTestRxRSSI(&rssi)) {
      test_result = false;
      PRINT("Failed to read RSSI\n");
    } else {
      PRINT("RSSI = %ddBm\n", (signed)rssi);
    }
    Delay(TICK_PER_SECOND);
  }
  RFTestRxStop();
  PRINT("RF RX test stopped\n");
  return test_result;
}

void HardwareTest(void) {
  bool test_result = true;
  PRINT("Myriota modem hardware test\n");
  GPIOSetModeInput(MODULE_BAND_PIN, GPIO_NO_PULL);
  if (GPIOGet(MODULE_BAND_PIN) == GPIO_HIGH) {
    PRINT("Testing VHF module %s\n", ModuleIDGet());
  } else {
    PRINT("Testing UHF module %s\n", ModuleIDGet());
  }
  PRINT("Test started\n");
  LedTurnOn();
  if (!GNSSTest(GNSS_COMM_TEST_ONLY) || !RFTXTest() || !RFRXTest()) {
    test_result = false;
    PRINT("Test failed\n");
  }
  LedTurnOff();
  PRINT("Test stopped\n");
  while (1) {
    if (!test_result) {
      Delay(250);
      LedToggle();
    }
  }
}
