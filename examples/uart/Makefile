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


PROGRAM_NAME = uart

ROOTDIR ?= $(abspath ../..)

APP_SRC = main.c

ifeq (sim, $(notdir $(PLATFORM)))
	APP_SRC += sim.c
else
	APP_SRC += $(BSP_PATH)/bsp.c
endif

include $(ROOTDIR)/terminal/app.mk
