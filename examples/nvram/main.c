// Copyright (c) 2021-2024, Myriota Pty Ltd, All Rights Reserved
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

// A simple application to demonstrate how to use NVRAM. The application saves
// the startup timestamp to NVRAM and clears the NVRAM when 2 timestamps have
// been saved.

#include "myriota_user_api.h"

NVRAM_MEM uint8_t NvramMem[NVRAM_MEM_SIZE];  // Can only be defined in one file

static time_t Nvram() {
  uint32_t timestamp;
  int num_of_timestamp;
  for (num_of_timestamp = 0; num_of_timestamp < 2; num_of_timestamp++) {
    timestamp = *(uint32_t *)(NvramMem + num_of_timestamp * sizeof(timestamp));
    if (timestamp != 0xFFFFFFFF)
      printf("Timestamp %d = %" PRIu32 "\n", num_of_timestamp + 1, timestamp);
    else
      break;
  }
  if (num_of_timestamp == 2) {
    NvramClear();
    printf("Clearing the records\n");
    num_of_timestamp = 0;
  }
  timestamp = TimeGet();
  NvramWrite(num_of_timestamp * sizeof(timestamp), (uint8_t *)&timestamp,
             sizeof(timestamp));
  return Never();
}

void AppInit() { ScheduleJob(Nvram, ASAP()); }
