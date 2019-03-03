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


include $(ROOTDIR)/terminal/sim/flags.mk
include $(ROOTDIR)/terminal/orbit_model.mk

LIB_DIR:=$(ROOTDIR)/terminal/sim
LIBS:=$(LIB_DIR)/sim.so
OBJ_LIST += $(orbit_model).o

$(PROGRAM_NAME) : $(LIBS) $(OBJ_LIST)
	$(CC) $(OBJ_LIST) $(LIBS) $(LDFLAGS) -o $@
	@rm $(ORBIT_MODEL) $(orbit_model).c $(orbit_model).o

clean:
	rm -f $(OBJ_LIST) $(PROGRAM_NAME)

.DEFAULT_GOAL:=$(PROGRAM_NAME)
