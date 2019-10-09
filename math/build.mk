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


allsourceindirs = $(foreach dir,$(1), $(foreach ext, $(2), $(wildcard $(dir)/*.$(ext))))

# default extentions
ASMEXT?=s
CEXT?=c
CPPEXT?=cpp

# get c and cpp sources and corresponding objects
MATH_BUILD_DIR := $(abspath .obj)
MATH_SRC_DIR := $(abspath $(ROOTDIR)/math)
MATH_SRC := $(call allsourceindirs,$(MATH_SRC_DIR) ,$(ASMEXT) $(CEXT) $(CPPEXT))
MATH_OBJECTS:=$(patsubst %,$(MATH_BUILD_DIR)%.mathlib.o,$(MATH_SRC))

## Build static myriotamath library
myriotamath.a : $(MATH_OBJECTS)
	ar rcs $@ $(MATH_OBJECTS)

# Header dependency files
MATH_DEPS := $(patsubst %.mathlib.o, %.mathlib.d, $(MATH_OBJECTS))

# Include header dependency files "-" ignores files that don't exist
-include $(MATH_DEPS)

# Targets for object files
# This allows separate build and source directories.
# MMD flag builds header dependecy files
$(MATH_BUILD_DIR)%.$(ASMEXT).mathlib.o : %.$(ASMEXT)
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(MATH_BUILD_DIR)%.$(CEXT).mathlib.o : %.$(CEXT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(MATH_BUILD_DIR)%.$(CPPEXT).mathlib.o : %.$(CPPEXT)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@
