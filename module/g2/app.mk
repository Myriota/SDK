# Copyright (c) 2016-2025, Myriota Pty Ltd, All Rights Reserved
# SPDX-License-Identifier: BSD-3-Clause-Attribution
#
# This file is licensed under the BSD with attribution  (the "License"); you
# may not use these files except in compliance with the License.
#
# You may obtain a copy of the License here:
# LICENSE-BSD-3-Clause-Attribution.txt and at
# https://spdx.org/licenses/BSD-3-Clause-Attribution.html
#
# See the License for the specific language governing permissions and
# limitations under the License.

export BOARD?=MyriotaDB

OBJ_DIR:=obj
NETWORK_INFO_DIR:=network_info
RAW_BINARY_DIR:=raw_binary
BUILD_WITH_NETWORKINFO?=0

include $(ROOTDIR)/module/g2/flags.mk
include $(ROOTDIR)/module/builtin.mk

PROGRAM_NAME?=app
PROGRAM_RAW_BIN:=$(PROGRAM_NAME)_raw.bin
PROGRAM_NAME_ELF:=$(PROGRAM_NAME).elf
APPLICATION_NETWORKINFO_BIN:=$(PROGRAM_NAME).bin

BSP_PATH?=$(ROOTDIR)/module/g2/boards/$(BOARD)

LAB_TEST ?= 0
LAB_TEST_WITH_LOCATION ?= 0

ifneq ($(LAB_TEST),0)
CFLAGS +=-DLAB_TEST
endif

ifneq ($(LAB_TEST_WITH_LOCATION),0)
CFLAGS +=-DLAB_TEST_WITH_LOCATION
endif

LIB_DIR:=$(ROOTDIR)/module/g2
LIBS:=$(LIB_DIR)/user_app_lib.a

$(shell mkdir -p $(OBJ_DIR))

# For backward compatibility
ifeq (,$(findstring bsp.c,$(APP_SRC)))
APP_SRC+=$(BSP_PATH)/bsp.c
endif
APP_OBJ:=$(patsubst %.c, $(OBJ_DIR)/%.o, $(APP_SRC))
SDK_OBJ:=$(builtin).o
OBJ_LIST+=$(APP_OBJ)
OBJ_LIST+=$(SDK_OBJ)

$(OBJ_DIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(PROGRAM_NAME): $(APPLICATION_NETWORKINFO_BIN)

$(APPLICATION_NETWORKINFO_BIN): $(OBJ_DIR)/$(PROGRAM_NAME_ELF) $(buildkey)
	$(OBJCOPY) -O binary $< $(PROGRAM_RAW_BIN)
	@(printf "0: "; xxd -ps -c32 $(buildkey)) | xxd -r - $(PROGRAM_RAW_BIN)
ifneq (,$(findstring $(BSP_PATH)/bsp.c,$(APP_SRC)))
	@echo "***Default BSP will be deprecated, please create BSP file (bsp.c) under the application folder***"
endif
ifeq (0, $(BUILD_WITH_NETWORKINFO))
	@mkdir -p $(RAW_BINARY_DIR);
	@cp $(NETWORK_INFO_DIR)/$(SATELLITES) $(RAW_BINARY_DIR)/$(LATEST_NETWORK_INFO).bin
	@mv $(PROGRAM_RAW_BIN) $(RAW_BINARY_DIR)/$(PROGRAM_RAW_BIN)
	$(ROOTDIR)/tools/merge_binary.py -n $(RAW_BINARY_DIR)/$(LATEST_NETWORK_INFO).bin -u $(RAW_BINARY_DIR)/$(PROGRAM_RAW_BIN) -o $@
else
	@mv $(PROGRAM_RAW_BIN) $(APPLICATION_NETWORKINFO_BIN)
endif
	@echo "Build has completed!"

$(OBJ_DIR)/$(PROGRAM_NAME_ELF): $(LIBS) $(OBJ_LIST)
	$(CC) $(OBJ_LIST) -Wl,--print-memory-usage -Wl,--whole-archive $(LIBS) $(LDFLAGS) -Wl,--no-whole-archive -o $@

clean:
	rm -f $(OBJ_LIST) *.bin $(PROGRAM_NAME_ELF)
	rm -rf $(OBJ_DIR) $(RAW_BINARY_DIR)

.DEFAULT_GOAL:=$(PROGRAM_NAME)

CPPEXT:=
CEXT:=c
ASMEXT:=s
