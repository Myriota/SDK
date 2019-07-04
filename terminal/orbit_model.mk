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
SATELLITES?=43215 SKA

ifeq (SATELLITES, "")
	SATELLITES:=/dev/null
endif

.PHONY: $(SATELLITES)
$(SATELLITES):
	curl -f https://static.myriota.com/access_nodes/r3/$@ --output $@ || \
		([ -e $@ ] && echo "Using old satellite $@") || \
		(echo "\e[33mError: Satellite $@ not found\e[0m"; exit 1)

orbit_model:=$(shell mktemp)

# create orbit_model.c source with updated orbit models
$(orbit_model).c: $(SATELLITES)
	printf "#include <inttypes.h>\n" > $@
	printf "const uint8_t* BuiltinAccessNodeGet() { static const uint8_t b[] = {" >> $@
	cat $^ | xxd -i | tr -d \\n >> $@
	printf "}; return b; }" >> $@
