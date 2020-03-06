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


remote_system_image_name=system_image_$(sed 's/\r//g;s/\./_/g' VERSION).bin
local_system_image_name=system.img

set -e
rm -f ${local_system_image_name}
tools/myriota_download.py -o ${local_system_image_name} ${remote_system_image_name}
echo "${local_system_image_name} for SDK $(sed 's/\r//g' VERSION) has been downloaded."
echo 'This file and the related binary are licensed under the Permissive Binary License, Version 1.0 (the "License"); you may not use these files except in compliance with the License.'
echo 'You may obtain a copy of the License here: LICENSE-permissive-binary-license-1.0.txt and at https://www.mbed.com/licenses/PBL-1.0'
echo 'See the License for the specific language governing permissions and limitations under the License.'
