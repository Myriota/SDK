# Copyright (c) 2016-2020, Myriota Pty Ltd, All Rights Reserved
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

OBJ_DIR:=obj
NETWORK_INFO_DIR:=network_info

include $(ROOTDIR)/terminal/sim/flags.mk
include $(ROOTDIR)/terminal/builtin.mk

$(shell mkdir -p $(OBJ_DIR))

LIB_DIR:=$(ROOTDIR)/terminal/sim
LIBS:=$(LIB_DIR)/sim.so
APP_OBJ:=$(patsubst %.c, $(OBJ_DIR)/%.o, $(APP_SRC))
SDK_OBJ:=$(builtin).o $(buildkey).o
OBJ_LIST+=$(APP_OBJ)
OBJ_LIST+=$(SDK_OBJ)

$(OBJ_DIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(PROGRAM_NAME) : $(OBJ_LIST) $(LIBS)
	$(CC) $(OBJ_LIST) $(LDFLAGS) $(LIBS) $(LDFLAGS) -o $@

clean:
	rm -f $(OBJ_LIST) $(PROGRAM_NAME)
	rm -rf $(OBJ_DIR)

.DEFAULT_GOAL:=$(PROGRAM_NAME)
