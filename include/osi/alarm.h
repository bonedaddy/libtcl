/*
 * Copyright 2018 Tempow
 *
 * Author - 2018 uael <abel@tempow.com>
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

/*!@file osi/alarm.h
 * @author uael
 *
 * @addtogroup osi.alarm @{
 */
#ifndef __OSI_ALARM_H
# define __OSI_ALARM_H

#include "osi/blocking_queue.h"
#include "osi/thread.h"
#include "osi/time.h"

typedef void (*alarm_callback_t)(void *data);

/*!@public
 *
 * @brief
 * The alarm structure declaration.
 */
typedef struct {
	const char *name;
	period_ms_t deadline;
	alarm_callback_t callback;
	period_ms_t creation_time;
	period_ms_t period;
	struct mutex callback_mutex;
	void *data;
	bool is_periodic;

//	uint64_t last_exec;
//	uint64_t interval;
//	work_t *work;
	blocking_queue_t* queue;  // The processing queue to add this alarm to
	struct list_head list_alarm;
} alarm_t;

/*!@public
 *
 * @brief
 * The alarm structure definition.
 */
struct alarm {
	const char *name;
	uint64_t last_exec;
	uint64_t interval;
	bool periodic;
	void *data;
	work_t *work;
};

typedef void (*alarm_callback_t)(void *data);
//typedef struct fixed_queue_t fixed_queue_t;

void alarm_cancel(alarm_t *alarm);
void alarm_set(alarm_t *alarm, period_ms_t interval_ms,
			   alarm_callback_t cb, void *data);
alarm_t *alarm_new(const char *name);
void alarm_free(alarm_t *alarm);
void alarm_set_on_queue(alarm_t *alarm, period_ms_t interval_ms,
						alarm_callback_t cb, void *data,
						fixed_queue_t *queue);
void alarm_register_processing_queue(fixed_queue_t *queue, thread_t *thread);
void alarm_unregister_processing_queue(fixed_queue_t *queue);
bool alarm_is_scheduled(const alarm_t *alarm);


#endif /* __OSI_ALARM_H */
/*!@} */
