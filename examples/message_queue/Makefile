# Copyright (c) 2025, Myriota Pty Ltd, All Rights Reserved
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

# Number of messages to be transmitted per day
MESSAGES_PER_DAY ?= 3

PROGRAM_NAME = message_queue

ROOTDIR ?= $(abspath ../..)

APP_SRC = main.c

ifneq (sim, $(notdir $(MODULE)))
	APP_SRC += bsp.c
endif

include $(ROOTDIR)/module/app.mk

CFLAGS+=-DMESSAGES_PER_DAY=$(MESSAGES_PER_DAY)
