// Copyright (c) 2016-2020, Myriota Pty Ltd, All Rights Reserved
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

#ifndef MYRIOTA_HARDWARE_SIM_API_H
#define MYRIOTA_HARDWARE_SIM_API_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup sim_apis simulator API
/// Simulator API
/// @{

/// ScheduleHook can be implemented to programmatically inject events, such as
/// GPIO wakeup into the simulation. ScheduleHook is called prior to each job
/// run. The "Next" argument is the time at which the next time based job will
// run. An event is injected prior to this job by returning a value smaller than
/// Next. A return value of 0 indicates no wakeup event.
time_t ScheduleHook(time_t Next);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // MYRIOTA_HARDWARE_SIM_API_H
