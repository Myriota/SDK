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


sudo apt-get -y update

# Myriota Development Board requirements
sudo apt-get -y install make curl python-pip
sudo -H pip install -r requirements.txt
curl -O https://static.myriota.com/gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
sudo mkdir -p /opt/gcc-arm
sudo tar -xjvf gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2 -C /opt/gcc-arm --strip-components=1
sudo cp module/g2/99-myriota-g2.rules /etc/udev/rules.d
sudo udevadm control --reload-rules && sudo udevadm trigger

# Satellite Simulator requirements
sudo apt-get -y install build-essential rtl-sdr
make -C tools/ satellite_simulator

# Tools for testing 
sudo apt-get -y install expect
