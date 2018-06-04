/*
 * Copyright (C) 2014 Google, Inc.
 * Copyright (C) 2018 Tempow
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/*!@file osi/time.h
 * @author uael
 *
 * @addtogroup osi.time @{
 */
#ifndef __OSI_TIME_H
# define __OSI_TIME_H

#include "osi/compat.h"

#ifdef HAS_TIME_H
# include <time.h>
#endif

typedef uint64_t period_ms_t;

/*!@public
 *
 * @brief
 * TODO
 *
 * @return
 */
period_ms_t now(void);

#define TIMER_INTERVAL_FOR_WAKELOCK_IN_MS 250

#define CLOCK_ID  CLOCK_BOOTTIME

#ifdef KERNEL_MISSING_CLOCK_BOOTTIME_ALARM
# define CLOCK_ID_ALARM CLOCK_BOOTTIME
#else
# define CLOCK_ID_ALARM CLOCK_BOOTTIME_ALARM
# undef HAS_TIMER
#endif

#endif /* __OSI_TIME_H */
/*!@} */
