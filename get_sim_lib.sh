#!/bin/bash
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


remote_sim_lib_name=sim_$(sed 's/\r//g;s/\./_/g' VERSION).so
local_sim_lib_name=terminal/sim/sim.so

set -e
rm -f ${local_sim_lib_name}
tools/myriota_download.py -o ${local_sim_lib_name} ${remote_sim_lib_name}
echo "${local_sim_lib_name} for SDK $(sed 's/\r//g' VERSION) has been downloaded."
echo 'This file and the related binary are licensed under the Permissive Binary License, Version 1.0 (the "License"); you may not use these files except in compliance with the License.'
echo 'You may obtain a copy of the License here: LICENSE-permissive-binary-license-1.0.txt and at https://www.mbed.com/licenses/PBL-1.0'
echo 'See the License for the specific language governing permissions and limitations under the License.'
