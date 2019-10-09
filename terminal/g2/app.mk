# Copyright (c) 2016-2019, Myriota Pty Ltd, All Rights Reserved
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

include $(ROOTDIR)/terminal/g2/flags.mk
include $(ROOTDIR)/terminal/builtin.mk

PROGRAM_NAME?=app
PROGRAM_NAME_BIN:=$(PROGRAM_NAME).bin
PROGRAM_NAME_ELF:=$(PROGRAM_NAME).elf

BSP_PATH?=$(ROOTDIR)/terminal/g2/boards/$(BOARD)

ifeq ($(SATELLITES),Lab)
CFLAGS +=-DLAB_TEST
endif

ifeq ($(SATELLITES),LabWithLocation)
CFLAGS +=-DLAB_TEST_WITH_LOCATION
endif

LIB_DIR:=$(ROOTDIR)/terminal/g2
LIBS:=$(LIB_DIR)/user_app_lib.a

# For backward compatibility
ifeq (,$(findstring bsp.c,$(APP_SRC)))
APP_SRC+=$(BSP_PATH)/bsp.c
endif
APP_OBJ:=$(patsubst %.c, %.o, $(APP_SRC))
SDK_OBJ:=$(builtin).o
OBJ_LIST+=$(APP_OBJ)
OBJ_LIST+=$(SDK_OBJ)

$(PROGRAM_NAME): $(PROGRAM_NAME_BIN)

$(PROGRAM_NAME_BIN): $(PROGRAM_NAME_ELF)
	arm-none-eabi-objcopy -O binary $< $@

$(PROGRAM_NAME_ELF): $(LIBS) $(OBJ_LIST)
	$(CC) $(OBJ_LIST) -Wl,--whole-archive $(LIBS) $(LDFLAGS) -Wl,--no-whole-archive -o $@

clean:
	rm -f $(OBJ_LIST) $(PROGRAM_NAME_BIN) $(PROGRAM_NAME_ELF)

.DEFAULT_GOAL:=$(PROGRAM_NAME)

CPPEXT:=
CEXT:=c
ASMEXT:=s
include $(ROOTDIR)/math/build.mk
