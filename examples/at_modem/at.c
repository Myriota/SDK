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

#include "at.h"
#include <ctype.h>
#include "at_defs.h"

// Comment out to disable debug print
#define AT_DEBUG

#ifdef AT_DEBUG
#define DEBUG_INFO(Format...) printf(Format)
#define DEBUG_ERROR(Format...) printf(Format)
#else
#define DEBUG_INFO(Format...)
#define DEBUG_ERROR(Format...)
#endif

static void *UartHandle = NULL;
static unsigned State = AT_STATE_INIT;

static int ASCIIToHex(char *Dest, const char *Src) {
  int char_cnt = 0;
  if (strlen(Src) == 0) return -1;
  for (int i = 0; i < strlen(Src); i++) {
    char ch = Src[i];
    if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
      if (char_cnt % 2 == 0) {  // Even position
        Dest[char_cnt / 2] = (ch <= '9' ? ch - '0' : ch - 'A' + 0xa) << 4;
      } else {  // Odd position
        Dest[char_cnt / 2] += (ch <= '9' ? ch - '0' : ch - 'A' + 0xa);
      }
      char_cnt++;
    } else {
      DEBUG_ERROR("Hex string contains illegal characters 0x%02x\n", ch);
      return -1;
    }
  }
  if (char_cnt % 2 != 0) {
    DEBUG_ERROR("Number of characters is not even\n");
    return -1;
  }
  return (char_cnt / 2);
}

int ATInit() {
  UartHandle = UARTInit(UART_INTERFACE, UART_BAUDRATE, 0);
  if (UartHandle == NULL) {
    DEBUG_ERROR("Failed to initialise uart\n");
    return -1;
  }
  return 0;
}

size_t ATReceive(char *Rx, size_t MaxLength) {
  const uint32_t start = TickGet();
  size_t count = 0;
  while ((TickGet() - start < RECEIVE_TIMEOUT) && (count <= MaxLength)) {
    uint8_t ch;
    if (UARTRead(UartHandle, &ch, 1) == 1) {
      if (count < MaxLength) {
        Rx[count++] = ch;
      }
    }
  }
  return count;
}

static int ATRespond(const char *Header, const char *Command,
                     const char *Parameter) {
  char Tx[UART_MAX_TX_SIZE + 1] = {0};
  if (Header != NULL) {
    strcpy(Tx, Header);
  }
  if (Command != NULL) {
    strcat(Tx, Command);
  }
  if (Parameter != NULL) {
    *(Tx + strlen(Tx)) = '=';
    strcat(Tx, Parameter);
  }
  strcat(Tx, AT_RESP_END);
  ATSend(Tx);
  return 0;
}

void ATSend(char *Tx) { UARTWrite(UartHandle, (uint8_t *)Tx, strlen(Tx)); }

void ATSetState(SysStates State) {
  switch (State) {
    case SYS_STATE_INIT:
      State = AT_STATE_INIT;
      break;
    case SYS_STATE_GNSS_ACQ:
      State = AT_STATE_GNSS_ACQ;
      break;
    case SYS_STATE_READY:
      State = AT_STATE_READY;
      break;
  }
  ATRespond(AT_STATE_START, NULL, States[State]);
}

static SysStates ATGetState() {
  if (HasValidGNSSFix()) State = AT_STATE_READY;
  return State;
}

static void QueryMsgQueueHandler(uint32_t CmdId) {
  char tx_para[] = "xxxx";
  int free_bytes = 0;
  free_bytes = MessageBytesFree();
  sprintf(tx_para, "%i", free_bytes);  // Decimal
  ATRespond(AT_RESP_OK_START, Queries[CmdId], tx_para);
  DEBUG_INFO("Free message bytes = %i\n", free_bytes);
}

static void QueryStateHandler(uint32_t CmdId) {
  ATRespond(AT_RESP_OK_START, Queries[CmdId], (char *)States[ATGetState()]);
  DEBUG_INFO("Read state = %s\n", (char *)States[ATGetState()]);
}

static void QuerySDKVersionHandler(uint32_t CmdId) {
  uint32_t major, minor, patch;
  char tx_para[] = "255.255.255";
  SDKVersionGet(&major, &minor, &patch);
  sprintf(tx_para, "%i.%i.%i", (int)major, (int)minor, (int)patch);
  ATRespond(AT_RESP_OK_START, Queries[CmdId], tx_para);
  DEBUG_INFO("SDK version is %i.%i.%i\n", (int)major, (int)minor, (int)patch);
}

static void QueryModuleIDHandler(uint32_t CmdId) {
  ATRespond(AT_RESP_OK_START, Queries[CmdId], ModuleIDGet());
  DEBUG_INFO("Read module ID = %s\n", ModuleIDGet());
}

static void QueryRegCodeHandler(uint32_t CmdId) {
  ATRespond(AT_RESP_OK_START, Queries[CmdId], RegistrationCodeGet());
  DEBUG_INFO("Read registration code = %s\n", RegistrationCodeGet());
}

static void QueryTimeHandler(uint32_t CmdId) {
  char tx_para[] = "1483228800";
  sprintf(tx_para, "%u", (unsigned int)TimeGet());
  ATRespond(AT_RESP_OK_START, Queries[CmdId], tx_para);
  DEBUG_INFO("Read time = %s\n", tx_para);
}

static void QueryLocationHandler(uint32_t CmdId) {
  char tx_para[] = "-900000000,-1800000000";
  int32_t lat, lon;
  LocationGet(&lat, &lon, NULL);
  sprintf(tx_para, "%i,%i", (signed)lat, (signed)lon);
  ATRespond(AT_RESP_OK_START, Queries[CmdId], tx_para);
  DEBUG_INFO("Read location = %s\n", tx_para);
}

static void QuerySuspendModeHandler(uint32_t CmdId) {
  char tx_para[] = "0";
  sprintf(tx_para, "%s", SuspendModeIsEnabled() ? "1" : "0");
  ATRespond(AT_RESP_OK_START, Queries[CmdId], tx_para);
  DEBUG_INFO("Suspend mode = %s\n", tx_para);
}

static void ControlSaveMsgHandler(uint32_t CmdId, const char *Para) {
  if (strlen(Para) == 0) {
    SaveMessages();
    ATRespond(AT_RESP_OK_START, Controls[CmdId], NULL);
    DEBUG_INFO("Save message\n");
  } else {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("Save message should not carry parameter\n");
  }
}

static uint32_t RFTxTimeout;
static void SetRFTxTimeout(uint32_t ms) { RFTxTimeout = TickGet() + ms; }
static uint32_t GetRFTxTimeout() { return RFTxTimeout; }

__attribute__((weak)) time_t KeepRFAwake() {
  if (TickGet() < GetRFTxTimeout())
    return ASAP();
  else {
    RFTestTxStop();
    ATRespond(AT_RESP_OK_START, Controls[AT_CONTROL_RF_TX_START], NULL);
    return Never();
  }
}

static void ControlTxStartHandler(uint32_t CmdId, const char *Para) {
  uint32_t tx_freq;
  uint8_t tx_type;
  bool tx_burst;
  bool invalid_para = false;
  if (strlen(Para) == 0) {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("RF TX test should have parameters\n");
  } else {  // Use specified parameters
    char buf[] = "400000000,0,0,999";
    if (strlen(Para) > strlen(buf)) {
      invalid_para = true;
    } else {
      strncpy(buf, Para, strlen(buf));
    }
    char *opt = NULL;
    if (invalid_para || (opt = strtok(buf, ",")) == NULL) {
      invalid_para = true;
    } else {
      tx_freq = (uint32_t)atoi(opt);
      if (tx_freq == 0) invalid_para = true;
    }
    if (invalid_para || (opt = strtok(NULL, ",")) == NULL) {
      invalid_para = true;
    } else {
      tx_type = (uint8_t)atoi(opt);
      if (tx_type != TX_TYPE_TONE && tx_type != TX_TYPE_PRBS) {
        invalid_para = true;
      }
    }
    if (invalid_para || (opt = strtok(NULL, ",")) == NULL) {
      invalid_para = true;
    } else {
      tx_burst = atoi(opt);
      if (tx_burst != 0 && tx_burst != 1) {
        invalid_para = true;
      }
    }
    uint32_t timeout;
    if (invalid_para || (opt = strtok(NULL, ",")) == NULL) {
      invalid_para = true;
    } else {
      timeout = atoi(opt) * 1000;
      if (timeout == 0 || timeout > RF_TX_TIMEOUT_MAX) {
        invalid_para = true;
      }
    }
    if (!invalid_para) {
      RFTestTxStop();
      if (RFTestTxStart(tx_freq, tx_type, tx_burst)) {
        ScheduleJob(KeepRFAwake, Never());
        ATRespond(AT_RESP_FAIL_START, Controls[CmdId], Para);
        DEBUG_ERROR("Radio transmit test failed\n");
      } else {
        ATRespond(AT_RESP_OK_START, Controls[CmdId], Para);
        DEBUG_INFO("Testing radio transmit: %i,%s,%s\n", (int)tx_freq,
                   (tx_type ? "TX_TYPE_PRBS" : "TX_TYPE_TONE"),
                   (tx_burst ? "true" : "false"));
        SetRFTxTimeout(timeout);
        ScheduleJob(KeepRFAwake, ASAP());
      }
    }
  }
  if (invalid_para) {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], Para);
    DEBUG_ERROR("Invalid parameter for RF TX start\n");
  }
}

static void ControlTxStopHandler(uint32_t CmdId, const char *Para) {
  if (strlen(Para) == 0) {
    ScheduleJob(KeepRFAwake, Never());
    RFTestTxStop();
    ATRespond(AT_RESP_OK_START, Controls[CmdId], NULL);
    DEBUG_INFO("Tx test stopped\n");
  } else {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("Stop RF TX should not carry parameter\n");
  }
}

static void ControlGnssFixHandler(uint32_t CmdId, const char *Para) {
  if (strlen(Para) == 0) {
    ATRespond(AT_RESP_OK_START, Controls[CmdId], NULL);
    if (GNSSFix()) {
      ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
      DEBUG_INFO("GNSSFIX failed\n");
      return;
    } else {
      ATRespond(AT_RESP_OK_START, Controls[CmdId], NULL);
      DEBUG_INFO("GNSSFIX successful\n");
    }
  } else {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("GNSSFIX should not carry parameter\n");
  }
}

static void ControlRssiHandler(uint32_t CmdId, const char *Para) {
  int32_t rssi;
  char resp[] = "400000000";
  if (strlen(Para) == 0) {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("No frequency specified for RSSI test\n");
  } else {
    uint32_t rx_freq = (uint32_t)atoi(Para);
    if (RFTestRxStart(rx_freq) || RFTestRxRSSI(&rssi)) {
      ATRespond(AT_RESP_FAIL_START, Controls[CmdId], Para);
      DEBUG_INFO("Failed to get RSSI @%iHz\n", (int)rx_freq);
    } else {
      snprintf(resp, strlen(resp), "%i", (int)rssi);
      ATRespond(AT_RESP_OK_START, Controls[CmdId], resp);
      DEBUG_INFO("RSSI = %idBm @%iHz\n", (int)rssi, (int)rx_freq);
    }
    RFTestRxStop();
  }
}

static void ControlScheduleMsgHandler(uint32_t CmdId, const char *Para) {
  int msg_len = 0;
  char msg[AT_MAX_PARA_LEN / 2 + 1] = {0};
  msg_len = ASCIIToHex(msg, Para);
  if (msg_len > MAX_MESSAGE_SIZE) {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId],
              ErrorCodes[AT_ERROR_MESSAGE_TOO_LONG]);
    DEBUG_ERROR("Message too long\n");
  }
  // Invalid parameter
  else if (msg_len <= 0) {
    ATRespond(AT_ERROR_START, NULL, ErrorCodes[AT_ERROR_INVALID_PARAMETER]);
    DEBUG_ERROR("Invalid parameter\n");
  } else {
    if (ScheduleMessage((uint8_t *)msg, msg_len) >= 0) {
      ATRespond(AT_RESP_OK_START, Controls[CmdId], Para);
      DEBUG_INFO("Scheduled message: ");
      for (int i = 0; i < msg_len; i++) {
        DEBUG_INFO("%02x", msg[i]);
      }
      DEBUG_INFO("\n");
    } else {
      ATRespond(AT_RESP_FAIL_START, Controls[CmdId], Para);
    }
  }
}

static void ControlSuspendMode(uint32_t CmdId, const char *Para) {
  if (strlen(Para) == 0) {
    ATRespond(AT_RESP_FAIL_START, Controls[CmdId], NULL);
    DEBUG_ERROR("No parameter speficified, 0 to enter and 1 to exit\n");
  } else {
    int enter_mode = atoi(Para);
    switch (enter_mode) {
      case 1:
        SuspendModeEnable(true);
        DEBUG_INFO("Suspend mode enabled\n");
        ATRespond(AT_RESP_OK_START, Controls[CmdId], Para);
        break;
      case 0:
        SuspendModeEnable(false);
        DEBUG_INFO("Suspend mode disabled\n");
        ATRespond(AT_RESP_OK_START, Controls[CmdId], Para);
        break;
      default:
        ATRespond(AT_RESP_FAIL_START, Controls[CmdId], Para);
        DEBUG_INFO("Unknown parameter %d,0 to enter and 1 to exit\n",
                   enter_mode);
        break;
    }
  }
}

static const QueryHandler_t QueryHandlers[] = {
    {AT_QUERY_MSG_QUEUE, &QueryMsgQueueHandler},
    {AT_QUERY_STATE, &QueryStateHandler},
    {AT_QUERY_SDK_VERSION, &QuerySDKVersionHandler},
    {AT_QUERY_MODULE_ID, &QueryModuleIDHandler},
    {AT_QUERY_REG_CODE, &QueryRegCodeHandler},
    {AT_QUERY_TIME, &QueryTimeHandler},
    {AT_QUERY_LOCATION, &QueryLocationHandler},
    {AT_QUERY_SUSPEND_MODE, &QuerySuspendModeHandler},
};

static const ControlHandler_t ControlHandlers[] = {
    {AT_CONTROL_SAVE_MESSAGE, &ControlSaveMsgHandler},
    {AT_CONTROL_RF_TX_START, &ControlTxStartHandler},
    {AT_CONTROL_RF_TX_STOP, &ControlTxStopHandler},
    {AT_CONTROL_GNSS_FIX, &ControlGnssFixHandler},
    {AT_CONTROL_RSSI, &ControlRssiHandler},
    {AT_CONTROL_SCHEDULE_MESSAGE, &ControlScheduleMsgHandler},
    {AT_CONTROL_SUSPEND_MODE, &ControlSuspendMode},
};

static void *GetHandlder(const char *CmdStr, const char *Strs[],
                         const CmdHandler_t *Handlers, unsigned CmdNum,
                         unsigned *CmdId) {
  BUILD_BUG_ON(AT_QUERY_NUM != NUM_ELEMS(Queries));
  BUILD_BUG_ON(AT_CONTROL_NUM != NUM_ELEMS(Controls));
  BUILD_BUG_ON(AT_ERROR_NUM != NUM_ELEMS(ErrorCodes));
  BUILD_BUG_ON(AT_QUERY_NUM != NUM_ELEMS(QueryHandlers));
  BUILD_BUG_ON(AT_CONTROL_NUM != NUM_ELEMS(ControlHandlers));

  void *Handler = NULL;
  for (unsigned int i = 0; i < CmdNum; i++) {
    if ((strlen(CmdStr) == strlen(Strs[i])) &&
        (memcmp(CmdStr, Strs[i], strlen(CmdStr)) == 0)) {
      for (unsigned int j = 0; j < CmdNum; j++) {
        if (Handlers[j].Id == i) {
          Handler = Handlers[j].Handler;
          *CmdId = i;
          break;
        }
      }
      if (Handler != NULL) break;
    }
  }
  return Handler;
}

static bool ProcessQuery(char *CmdStr) {
  unsigned CmdId;
  void *Handler = GetHandlder(CmdStr, Queries, (CmdHandler_t *)QueryHandlers,
                              AT_QUERY_NUM, &CmdId);
  if (Handler == NULL) return false;
  ((QueryHandlerFunc_t)Handler)(CmdId);
  return true;
}

static bool ProcessControl(char *CmdStr, const char *Para) {
  unsigned CmdId;
  void *Handler = GetHandlder(CmdStr, Controls, (CmdHandler_t *)ControlHandlers,
                              AT_CONTROL_NUM, &CmdId);
  if (Handler == NULL) return false;
  ((ControlHandlerFunc_t)Handler)(CmdId, Para);
  return true;
}

static int ATCmdProcess(char *Cmd, const char *Para) {
  bool ret_query, ret_control;
  if (memcmp(Para, AT_QUERY, strlen(AT_QUERY)) == 0) {
    ret_query = ProcessQuery(Cmd);
    if (!ret_query) {
      DEBUG_ERROR("Unknown query command\n");
      ATRespond(AT_ERROR_START, NULL, ErrorCodes[AT_ERROR_UNKNOWN_QUERY_CMD]);
    }
  } else {
    ret_control = ProcessControl(Cmd, Para);
    if (!ret_control) {
      DEBUG_ERROR("Unknown control command\n");
      ATRespond(AT_ERROR_START, NULL, ErrorCodes[AT_ERROR_UNKNOWN_CONTROL_CMD]);
    }
  }
  return 0;
}

void ATProcess(char *Input, int Len) {
  bool invalid_cmd;
  char *input_start = Input;
  char *cmd_start = Input, *cmd_end = Input;
  const char *input_end = Input + Len - 1;
  // Process all commands in the input string
  while (input_start <= input_end) {
    invalid_cmd = false;
    // Find the start of current command
    cmd_start = input_start;
    while (isspace(*cmd_start) && (cmd_start < input_end)) {
      cmd_start++;
    }
    // Find the end of current command
    cmd_end = cmd_start;
    while ((!isspace(*cmd_end)) && (cmd_end < input_end)) {
      cmd_end++;
    }
    // No more commands
    if (cmd_start == cmd_end) break;
    if (!isspace(*cmd_end)) {
      invalid_cmd = true;
      DEBUG_ERROR("No terminator\n");
    } else if (cmd_end - cmd_start < AT_MIN_RX_SIZE) {
      invalid_cmd = true;
      DEBUG_ERROR("Command too short\n");
    }
    if (invalid_cmd == false) {
      *cmd_end = '\0';
      if (memcmp(cmd_start, AT_AT, strlen(AT_AT)) == 0) {
        // Handle command "AT" straightaway
        if (strlen(cmd_start) == strlen(AT_AT)) {
          ATRespond("OK", NULL, NULL);
          DEBUG_INFO("Communication check\n");
          if (cmd_end == input_end) break;
          input_start = cmd_end + 1;
          continue;
        }
        char *cmd = cmd_start + strlen(AT_CMD_START);
        char para[AT_MAX_PARA_LEN + 1] = {0};
        if (strstr(cmd, "=") != NULL) {
          strtok(cmd, "=");
          if (strlen(cmd) > AT_MAX_CMD_LEN) {
            invalid_cmd = true;
            DEBUG_ERROR("Command too long\n");
          }
          char *p = strtok(NULL, "=");
          if (strlen(p) > AT_MAX_PARA_LEN) {
            invalid_cmd = true;
            DEBUG_ERROR("Parameter too long\n");
          } else {
            strcpy(para, p);
          }
        }
        if (invalid_cmd == false) {
          ATCmdProcess(cmd, para);
        }
      } else {
        invalid_cmd = true;
        DEBUG_ERROR("No header\n");
      }
    }
    if (invalid_cmd == true) {
      ATRespond(AT_ERROR_START, NULL, ErrorCodes[AT_ERROR_INVALID_COMMAND]);
    }
    if (cmd_end == input_end) break;
    input_start = cmd_end + 1;
  }
}
