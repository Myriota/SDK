// Copyright (c) 2022, Myriota Pty Ltd, All Rights Reserved
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

// This example demonstrates how to retrieve diagnostics information
// via system update APIs.

#include <string.h>
#include "myriota_user_api.h"

#define TRANSFER_BLOCK_SIZE 128

static void DumpMessage(uint8_t *buf, int buflen) {
  printf("DiagnosticsReadXfer: ");
  for (int i = 0; i < buflen; i++) printf("%02x", buf[i]);
  printf("\n");
}

static int DiagnosticsReadXfer(uint32_t offset) {
  uint8_t buf[TRANSFER_BLOCK_SIZE];
  int ret = SystemUpdateXfer(offset, buf, TRANSFER_BLOCK_SIZE);

  if (ret > 0) {
    DumpMessage(buf, ret);

    // @TODO: handle read data on successful read
  }

  return ret;
}

static time_t DiagnosticsReadTask(void) {
  const int info_size = SystemUpdateStart(SYSTEM_GET_ID_DIAGNOSTICS, 0, 20);
  if (info_size <= 0) {
    printf("*** Error starting diagnostics snapshot read: ret %d\n", info_size);
    return SecondsFromNow(5);
  }

  printf("DiagnosticsRead started\n");

  uint32_t bytes_read = 0;
  int ret = 0;
  while (bytes_read < info_size) {
    if (bytes_read < info_size) ret = DiagnosticsReadXfer(bytes_read);
    if (ret <= 0) break;

    bytes_read += ret;
    if (bytes_read >= info_size) break;
  }

  ret = SystemUpdateFinish();
  if (ret < 0 || bytes_read < info_size) {
    printf("DiagnosticsReadFinish failed\n");
    return SecondsFromNow(5);
  }

  printf("DiagnosticsReadFinish succeeded with %s read size %d\n",
         bytes_read == info_size ? "matched" : "unmatched", info_size);
  return Never();
}

void AppInit() { ScheduleJob(DiagnosticsReadTask, SecondsFromNow(5)); }
