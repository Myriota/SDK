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
include $(ROOTDIR)/terminal/orbit_model.mk

PROGRAM_NAME?=app
PROGRAM_NAME_BIN:=$(PROGRAM_NAME).bin
PROGRAM_NAME_ELF:=$(PROGRAM_NAME).elf

BSP_PATH:=$(ROOTDIR)/terminal/g2/boards/$(BOARD)

ifeq ($(SATELLITES),Lab)
CFLAGS +=-DLAB_TEST
endif

LIB_DIR:=$(ROOTDIR)/terminal/g2
LIBS:=$(LIB_DIR)/user_app_lib.a

OBJ_LIST += $(orbit_model).o $(BSP_PATH)/bsp.o

$(PROGRAM_NAME): $(PROGRAM_NAME_BIN)

$(PROGRAM_NAME_BIN): $(PROGRAM_NAME_ELF)
	arm-none-eabi-objcopy -O binary $< $@

$(PROGRAM_NAME_ELF): $(LIBS) $(OBJ_LIST)
	$(CC) $(OBJ_LIST) -Wl,--whole-archive $(LIBS) $(LDFLAGS) -Wl,--no-whole-archive -o $@

# Specify the serial port using the -p option if it is different from the default port
# chosen by updater.py. See updater.py --help
run: $(PROGRAM_NAME_BIN)
	updater.py -f $< -s

clean:
	rm -f $(OBJ_LIST) $(PROGRAM_NAME_BIN) $(PROGRAM_NAME_ELF)

.DEFAULT_GOAL:=$(PROGRAM_NAME)

CPPEXT:=
CEXT:=c
ASMEXT:=s
include $(ROOTDIR)/math/build.mk
