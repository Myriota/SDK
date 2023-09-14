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


myriota_test() {
    local mlen=${#1}
    local test_string_len=90
    printf "Myriota test: ${1} "
    for i in `seq $mlen $test_string_len`; do printf "."; done;
    printf " "
    testlog=$(mktemp)
    bash -c "${1}" > testlog 2>&1 && echo "pass" || (echo "FAIL"; cat testlog)
}

myriota_test 'which make'
myriota_test 'which python'
myriota_test 'which pip || which pip3'
myriota_test 'python -c "import serial"'
myriota_test 'python -c "import requests"'
myriota_test 'python -c "from OpenSSL import crypto"'
myriota_test 'which curl'
myriota_test 'arm-none-eabi-gcc --version | grep -q 7.2.1'
myriota_test 'which gcc || which clang'
myriota_test 'which rtl_sdr'
myriota_test 'which expect'
myriota_test 'updater.py --help'
myriota_test 'log-util.py --help'
myriota_test 'myriota_auth.py --help'
myriota_test 'merge_binary.py --help'
myriota_test 'convert_type --help'
myriota_test 'resample --help'
myriota_test 'satellite_simulator.py --help'
myriota_test 'cd examples/blinky; make; make clean'
myriota_test 'cd examples/tracker; make; make clean'
myriota_test 'examples/tracker/unpack.py -h'
myriota_test 'cd examples/event; make; make clean'
