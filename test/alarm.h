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

#ifndef __OSI_TEST_ALARM_H
# define __OSI_TEST_ALARM_H

#include "test.h"

#include "osi/alarm.h"
#include "osi/blocking_queue.h"
#include "osi/sema.h"
#include "osi/log.h"
#include "osi/thread.h"

#define PTR_TO_INT(p) ((int) ((intptr_t) (p)))
#define INT_TO_PTR(i) ((void *) ((intptr_t) (i)))

#define EPSILON_MS 5

static sema_t semaphore;
static sema_t semaphore2;
static int cb_counter;
static int cb_counter2;
static int cb_misordered_counter;

static inline void cb(void *data)
{
	(void) data;
	++cb_counter;
	printf("counter: %d\n", cb_counter);
	sema_post(&semaphore);
}
static inline void cb2(void *data)
{
	static int count = 0;
	(void) data;
	printf("INSIDE cb2: %d\n", ++count);
	sema_wait(&semaphore2);
	++cb_counter2;
	printf("counter2: %d\n", cb_counter2);
}
static inline void ordered_cb(void *data)
{
	int i = PTR_TO_INT(data);
	if (i != cb_counter)
		cb_misordered_counter++;
	++cb_counter;
	printf("counter: %d\n", cb_counter);
	sema_post(&semaphore);
}

static inline void msleep(uint64_t ms)
{
	usleep((__useconds_t)(ms * 1000));
}

#endif /* __OSI_TEST_ALARM_H */
