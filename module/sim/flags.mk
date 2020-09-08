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
CC = gcc
CFLAGS = -std=gnu99 -g -Wall -Werror -fsanitize=address -I$(ROOTDIR) -I$(ROOTDIR)/module/include -I. -I$(ROOTDIR)/module/$(MODULE)/include -DDEBUG
LDFLAGS = -lasan -lm -lcrypto -Wall -Werror -fdata-sections -ffunction-sections
