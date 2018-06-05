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

#define LOG_TAG "bt_tcl_time"

#include "tcl/time.h"
#include "tcl/log.h"

#ifdef OS_MAC
# include <mach/clock.h>
# include <mach/mach.h>
#endif

static int __gettime(struct timespec *ts)
{
#ifdef OS_MAC
	clock_serv_t cclock;
	mach_timespec_t mts;

	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	if (clock_get_time(cclock, &mts))
		return -1;
	mach_port_deallocate(mach_task_self(), cclock);
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;
	retutn 0;
#else
	return clock_gettime(CLOCK_REALTIME, ts);
#endif
}

period_ms_t now(void)
{
	struct timespec ts;

	if (__gettime(&ts) == -1) {
		LOG_ERROR("Unable to get current time: %m");
		return 0;
	}
	return (period_ms_t) ((ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL));
}
