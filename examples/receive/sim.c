// Copyright (c) 2023, Myriota Pty Ltd, All Rights Reserved
// SPDX-License-Identifier: BSD-3-Clause-Attribution
//
// This file is licensed under the BSD with attribution  (the "License"); you
// may not use these files except in compliance with the License.
//
// You may obtain a copy of the License here:
// LICENSE-BSD-3-Clause-Attribution.txt and at
// https://spdx.org/licenses/BSD-3-Clause-Attribution.html
//
// See the License for the specific language governing permissions and
// limitations under the License.

// The simulation code for the at_modem example. This is only included when
// building the application for the host simulator.

#include <string.h>
#include <unistd.h>

#ifndef GNSS_SIM_FIX_STATUS
// 0-success, 1-fail
#define GNSS_SIM_FIX_STATUS 0
#endif

int GNSSFix(void) { return GNSS_SIM_FIX_STATUS; }
