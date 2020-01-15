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

# Satellites to be used
SATELLITES?=DefaultNetworkInfo

ifeq (SATELLITES, "")
	SATELLITES:=/dev/null
endif

ACCESS_NODES_BASEURL?=https://static.myriota.com/access_nodes
ACCESS_NODE_REVISION?=r9

.PHONY: $(SATELLITES)
$(SATELLITES):
	curl -f $(ACCESS_NODES_BASEURL)/$(ACCESS_NODE_REVISION)/$@ --output $@ || \
		([ -e $@ ] && echo "Using old satellite $@") || \
		(echo "\e[33mError: Satellite $@ not found\e[0m"; exit 1)

builtin:=$(shell mktemp)
buildkey:=$(shell mktemp -u)

$(buildkey):
	openssl rand 16 > $@
	cat /dev/zero | head -c16 >> $@
	printf "BuildKey: "; cat $@ | xxd -ps -c32

# create built-in source with updated orbit models
$(builtin).c: $(SATELLITES)
	printf "#include <inttypes.h>\n" > $@
	printf "const uint8_t* BuiltinNetworkInfo() { static const uint8_t b[] = {" >> $@
	dd if=/dev/zero bs=1 count=1 status=none | cat $(SATELLITES) - | xxd -i | tr -d \\n >> $@
	printf "}; return b; }\n" >> $@

# create build key source for simulation
$(buildkey).c: $(buildkey)
	printf "#include <inttypes.h>\n" > $@
	printf "const uint8_t* BuildKey() { static const uint8_t k[] = {" >> $@	
	cat $(buildkey) | xxd -i | tr -d \\n >> $@	
	printf "}; return k; }" >> $@

