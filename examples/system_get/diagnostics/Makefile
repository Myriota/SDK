# Copyright (c) 2022, Myriota Pty Ltd, All Rights Reserved
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


PROGRAM_NAME = diagnostics_get

ROOTDIR ?= $(abspath ../../..)

APP_SRC = main.c

ifeq (sim, $(notdir $(MODULE)))
	APP_SRC += sim.c
else
	APP_SRC += bsp.c
endif

include $(ROOTDIR)/module/app.mk
