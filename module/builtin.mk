# Copyright (c) 2016-2021, Myriota Pty Ltd, All Rights Reserved
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

# Satellites to be used
SATELLITES?=DefaultNetworkInfo
BUILD_WITH_NETWORKINFO?=1

ifeq ($(SATELLITES), )
	SATELLITES:=/dev/null
endif

ACCESS_NODES_BASEURL?=https://static.myriota.com/access_nodes
ACCESS_NODE_REVISION?=r10

LATEST_NETWORK_INFO:=networkinfo

ifeq ($(NETWORK_INFO_DIR), )
NETWORK_INFO:=$(SATELLITES)
else
NETWORK_INFO:=$(patsubst %,$(NETWORK_INFO_DIR)/%,$(SATELLITES))
endif
.PHONY: $(NETWORK_INFO)
$(NETWORK_INFO):
	@if [ -n "$(NETWORK_INFO_DIR)" ]; then \
	mkdir -p $(NETWORK_INFO_DIR); \
	fi;
	@curl -s -f -m 3 $(ACCESS_NODES_BASEURL)/$(ACCESS_NODE_REVISION)/$(notdir $@) --output $@ || \
		([ -e $@ ] && echo "Using old satellite $(notdir $@)") || (echo "\e[33mError: Satellite $(notdir $@) not found\e[0m"; exit 1)

.PHONY: $(LATEST_NETWORK_INFO)
$(LATEST_NETWORK_INFO):
	@(curl -s -f $(ACCESS_NODES_BASEURL)/$(ACCESS_NODE_REVISION)/$(SATELLITES) -o $@.bin && echo "Latest network information has been downloaded to $@.bin.") || \
	echo "Failed to download latest network information."

builtin:=$(shell mktemp)
buildkey:=$(shell mktemp -u)

SDK_VERSION?=$(shell cat $(ROOTDIR)/VERSION)

$(buildkey):
	@printf "0: %08x" $$(date +%s) | xxd -r - $@
	@echo $(SDK_VERSION) | awk -F"." '{printf ("4: %02x%02x%02x", $$1,$$2,$$3)}' | xxd -r - $@
	@openssl rand 9 >> $@
	@cat /dev/zero | head -c16 >> $@
	@printf "BuildKey: "; cat $@ | xxd -ps -c32

# create built-in source with updated orbit models
$(builtin).c: $(NETWORK_INFO)
	@printf "#include <inttypes.h>\n#include <stddef.h>\n" > $@
ifeq ($(BUILD_WITH_NETWORKINFO), 1)
	@printf "const uint8_t* BuiltinNetworkInfo() { static const uint8_t b[] = {" >> $@
	@dd if=/dev/zero bs=1 count=1 status=none | cat $^ - | xxd -i | tr -d \\n >> $@
	@printf "}; return b; }\n" >> $@
else
	@printf "const uint8_t* BuiltinNetworkInfo() { return NULL; }\n" >> $@
endif

# create build key source for simulation
$(buildkey).c: $(buildkey)
	@printf "#include <inttypes.h>\n" > $@
	@printf "const uint8_t* BuildKey() { static const uint8_t k[] = {" >> $@
	@cat $(buildkey) | xxd -i | tr -d \\n >> $@
	@printf "}; return k; }" >> $@
