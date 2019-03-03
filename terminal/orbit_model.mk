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
SATELLITES?=43215

# orbit model url
orbitmodelurl:=https://static.myriota.com/access_nodes/r0

$(SATELLITES): .FORCE
	curl -Of $(orbitmodelurl)/$@ || echo "Invalid satellite"

ifndef ORBIT_MODEL
ORBIT_MODEL:=$(shell mktemp)
$(ORBIT_MODEL) : $(SATELLITES)
	cat $(SATELLITES) > $@
	rm -f $(SATELLITES)
else
$(ORBIT_MODEL) :
endif

orbit_model:=$(ORBIT_MODEL)_SRC

## create orbit_model.c source with updated orbit model
$(orbit_model).c: $(ORBIT_MODEL)
	printf "static const char *hex = \"" > $@
	cat $(ORBIT_MODEL) | xxd -p | tr -d \\n >> $@
	printf "\";\n\n" >> $@
	printf "const char* orbit_model() { return hex; }\n" >> $@

.FORCE:
