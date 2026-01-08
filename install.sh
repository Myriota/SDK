#!/bin/bash
# Copyright (c) 2016-2025, Myriota Pty Ltd, All Rights Reserved
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

CROSS_COMPILER_PATH=/opt/gcc-arm-13_2_1

if [[ $OSTYPE == 'linux'* ]]; then
  echo "SDK Installation on Linux System"
  sudo apt-get -y update

  # Myriota Development Board requirements
  if $(cat /etc/os-release | grep -q "Ubuntu 2[2-9]\.04"); then
    sudo apt-get -y install make curl python3 python3-pip python-is-python3 python3-venv
    python3 -m venv ~/.venvs/myriota_sdk
    source ~/.venvs/myriota_sdk/bin/activate
    pip3 install -r requirements.txt
  else
    echo -e "\t======================================================================="
    echo -e "\tWARNING: Ubuntu 20.04 and under are not supported."
    echo -e "\t======================================================================="
  fi
  curl -O https://downloads.myriota.com/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
  sudo mkdir -p ${CROSS_COMPILER_PATH}
  sudo tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz -C ${CROSS_COMPILER_PATH} --strip-components=1
  sudo cp module/99-myriota.rules /etc/udev/rules.d
  sudo udevadm control --reload-rules && sudo udevadm trigger

  # Tools useful for host simulator
  sudo apt-get -y install build-essential

  # Tools for testing
  sudo apt-get -y install expect
elif [[ $OSTYPE == 'darwin'* ]]; then
  echo "SDK Installation on macOS"

  # Myriota Development Board requirements
  python3 -m venv ~/.venvs/myriota_sdk
  source ~/.venvs/myriota_sdk/bin/activate
  pip3 install -r requirements.txt

  echo "Install ARM compiler"
  if [[ $(uname -m) == 'arm64' ]]; then
    curl -O https://downloads.myriota.com/arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi.tar.xz
    sudo mkdir -p ${CROSS_COMPILER_PATH}
    sudo tar -xf arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi.tar.xz -C ${CROSS_COMPILER_PATH} --strip-components=1
  else
    curl -O https://downloads.myriota.com/arm-gnu-toolchain-13.2.rel1-darwin-x86_64-arm-none-eabi.tar.xz
    sudo mkdir -p ${CROSS_COMPILER_PATH}
    sudo tar -xf arm-gnu-toolchain-13.2.rel1-darwin-x86_64-arm-none-eabi.tar.xz -C ${CROSS_COMPILER_PATH} --strip-components=1
  fi

fi
