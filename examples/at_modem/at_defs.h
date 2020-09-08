// Copyright (c) 2020, Myriota Pty Ltd, All Rights Reserved
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

// This file can be included by host C application to simplify the
// implementation

#ifndef AT_DEFS_H
#define AT_DEFS_H

#define AT_AT "AT"
#define AT_CMD_START "AT+"
#define AT_RESP_OK_START "OK+"
#define AT_RESP_FAIL_START "FAIL+"
#define AT_STATE_START "+STATE"
#define AT_ERROR_START "ERROR"
#define AT_QUERY "?"
#define AT_RESP_END "\r\n"

#define AT_MAX_CMD_LEN 10
#define AT_MAX_PARA_LEN (MAX_MESSAGE_SIZE * 2)
#define AT_MIN_RX_SIZE (strlen(AT_AT))

enum {
  AT_QUERY_MSG_QUEUE = 0,
  AT_QUERY_STATE,
  AT_QUERY_SDK_VERSION,
  AT_QUERY_MODULE_ID,
  AT_QUERY_REG_CODE,
  AT_QUERY_TIME,
  AT_QUERY_LOCATION,
  AT_QUERY_NUM,
};

enum {
  AT_CONTROL_SAVE_MESSAGE = 0,
  AT_CONTROL_RF_TX_START,
  AT_CONTROL_RF_TX_STOP,
  AT_CONTROL_GNSS_FIX,
  AT_CONTROL_RSSI,
  AT_CONTROL_SCHEDULE_MESSAGE,
  AT_CONTROL_NUM,
};

enum {
  AT_ERROR_INVALID_PARAMETER = 0,
  AT_ERROR_MESSAGE_TOO_LONG,
  AT_ERROR_BUFFER_OVERFLOW,
  AT_ERROR_UNKNOWN_QUERY_CMD,
  AT_ERROR_UNKNOWN_CONTROL_CMD,
  AT_ERROR_INVALID_COMMAND,
  AT_ERROR_NUM,
};

enum { AT_STATE_INIT, AT_STATE_GNSS_ACQ, AT_STATE_READY, AT_STATE_UNKNOWN };

static const char* Queries[] = {
    "MSGQ", "STATE", "VSDK", "MID", "REGCODE", "TIME", "LOCATION",
};

static const char* Controls[] = {
    "SAVEMSG", "TXSTART", "TXSTOP", "GNSSFIX", "RSSI", "SMSG",
};

static const char* ErrorCodes[] = {
    "INVALID_PARAMETER", "MESSAGE_TOO_LONG",    "BUFFER_OVERFLOW",
    "UNKNOWN_QUERY_CMD", "UNKNOWN_CONTROL_CMD", "INVALID_COMMAND",
};

static const char* States[] = {
    "INITIALIZING",
    "GNSS_ACQ",
    "READY",
    "UNKNOWN",
};

#endif
