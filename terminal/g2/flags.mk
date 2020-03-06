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

##Default c compiler and flags
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
CFLAGS = -Wall -Werror -mcpu=cortex-m4 -mthumb -ffunction-sections -fdata-sections -fomit-frame-pointer -Os -I$(ROOTDIR) -I$(ROOTDIR)/terminal/include -I. -I$(ROOTDIR)/terminal/$(PLATFORM)/include -std=gnu99
LDSCRIPT =$(ROOTDIR)/terminal/g2/ldscript/APP.ld

LDFLAGS = -Wl,-no-wchar-size-warning -Wl,-Map=$(OBJ_DIR)/map.out -Wall -Werror -mcpu=cortex-m4 -mlittle-endian -mthumb -fdata-sections -ffunction-sections -T$(LDSCRIPT) -lm -Wl,--gc-sections -Xlinker -static -specs=nano.specs -u _printf_float -u _scanf_float
