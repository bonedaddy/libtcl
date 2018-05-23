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

#define LOG_TAG "alarm"

#ifdef HAS_TIMER
#include <signal.h>
#include <time.h>
#endif
#include <osi/alarm.h>
#include "osi/list.h"
#include "osi/log.h"
#include "osi/alarm.h"
#include "osi/mutex.h"
#include "osi/thread.h"


#define THREAD_RT_PRIORITY 1

static bool init_started = false;
static bool is_ready = false;// could remplace dispatcher_thread_active

static LIST_HEAD(alarms);
static struct mutex alarms_mutex;

#ifdef HAS_TIMER
static timer_t timer;
static timer_t wakeup_timer;
#endif

static bool dispatcher_thread_active = false;
static thread_t dispatcher_thread;

static thread_t default_callback_thread;
static blocking_queue_t default_callback_queue;

static sema_t alarm_expired;

static inline bool is_first_alarm(alarm_t *alarm) {
	return !list_empty(&alarms) &&
		   list_first_entry(&alarms, alarm_t, list_alarm) == alarm;
}

static void remove_pending_alarm(alarm_t *alarm) {
	list_del_init(&alarm->list_alarm);
	//TODO remove from alarm->queue
//	while (fixed_queue_try_remove_from_queue(alarm->queue, alarm) != NULL) {
	// Remove all repeated alarm instances from the queue.
	// NOTE: We are defensive here - we shouldn't have repeated alarm instances
//	}
}

#ifdef HAS_TIMER

// NOTE: must be called with |alarms_mutex| held
static bool timer_set = false;

static void reschedule_root_alarm(void) {
#ifdef HAS_WAKELOCK
	const bool timer_was_set = timer_set;
#endif
	// If used in a zeroed state, disarms the timer.
	struct itimerspec timer_time;

	memset(&timer_time, 0, sizeof(timer_time));
	if (list_empty(&alarms))
		goto done;
	const period_ms_t next_deadline = list_first_entry(&alarms, alarm_t, list_alarm)->deadline;
	const int64_t next_expiration = next_deadline - now();
	if (next_expiration < TIMER_INTERVAL_FOR_WAKELOCK_IN_MS) {
#ifdef HAS_WAKELOCK
		if (!timer_set) {
			if (!wakelock_acquire()) {
				LOG_ERROR(LOG_TAG, "%s unable to acquire wake lock", __func__);
				goto done;
			}
		}
#endif
		timer_time.it_value.tv_sec = (next_deadline / 1000);
		timer_time.it_value.tv_nsec = (next_deadline % 1000) * 1000000LL;
		// It is entirely unsafe to call timer_settime(2) with a zeroed timerspec
		// for timers with *_ALARM clock IDs. Although the man page states that the
		// timer would be canceled, the current behavior (as of Linux kernel 3.17)
		// is that the callback is issued immediately. The only way to cancel an
		// *_ALARM timer is to delete the timer. But unfortunately, deleting and
		// re-creating a timer is rather expensive; every timer_create(2) spawns a
		// new thread. So we simply set the timer to fire at the largest possible
		// time.
		//
		// If we've reached this code path, we're going to grab a wake lock and
		// wait for the next timer to fire. In that case, there's no reason to
		// have a pending wakeup timer so we simply cancel it.
		struct itimerspec end_of_time;
		memset(&end_of_time, 0, sizeof(end_of_time));
		end_of_time.it_value.tv_sec = (time_t) (1LL << (sizeof(time_t) * 8 - 2));
		timer_settime(wakeup_timer, TIMER_ABSTIME, &end_of_time, NULL);
	} else {
		// WARNING: do not attempt to use relative timers with *_ALARM clock IDs
		// in kernels before 3.17 unless you have the following patch:
		// https://lkml.org/lkml/2014/7/7/576
		struct itimerspec wakeup_time;
		memset(&wakeup_time, 0, sizeof(wakeup_time));
		wakeup_time.it_value.tv_sec = (next_deadline / 1000);
		wakeup_time.it_value.tv_nsec = (next_deadline % 1000) * 1000000LL;
		if (timer_settime(wakeup_timer, TIMER_ABSTIME, &wakeup_time, NULL) ==
			-1)
			LOG_ERROR("Unable to set wakeup timer: %m");
	}
	done:
	timer_set =
		timer_time.it_value.tv_sec != 0 || timer_time.it_value.tv_nsec != 0;
#ifdef HAS_WAKELOCK
	if (timer_was_set && !timer_set) {
		wakelock_release();
	}
#endif
	if (timer_settime(timer, TIMER_ABSTIME, &timer_time, NULL) == -1)
		LOG_ERROR("Unable to set timer: %m");
	// If next expiration was in the past (e.g. short timer that got context
	// switched) then the timer might have diarmed itself. Detect this case and
	// work around it by manually signalling the |alarm_expired| semaphore.
	//
	// It is possible that the timer was actually super short (a few
	// milliseconds) and the timer expired normally before we called
	// |timer_gettime|. Worst case, |alarm_expired| is signaled twice for that
	// alarm. Nothing bad should happen in that case though since the callback
	// dispatch function checks to make sure the timer at the head of the list
	// actually expired.
	if (timer_set) {
		struct itimerspec time_to_expire;
		timer_gettime(timer, &time_to_expire);
		if (time_to_expire.it_value.tv_sec == 0 &&
			time_to_expire.it_value.tv_nsec == 0) {
			LOG_DEBUG("Alarm expiration too close for posix timers, switching to guns");
			sema_post(&alarm_expired);
		}
	}
}
#else
static void reschedule_root_alarm(void) {
	//nothing to do
}
#endif

static void schedule_next_instance(alarm_t *alarm) {
	period_ms_t just_now;
	period_ms_t ms_into_period;
	alarm_t *alarm_entry;
	bool needs_reschedule;

	needs_reschedule = is_first_alarm(alarm);
	if (alarm->callback)
		remove_pending_alarm(alarm);
	ms_into_period = 0;
	just_now = now();
	if (alarm->is_periodic && alarm->period != 0)
		ms_into_period = ((just_now - alarm->creation_time) % alarm->period);
	// Calculate the next deadline for this alarm
	alarm->deadline = just_now + (alarm->period - ms_into_period);
	// Add it into the timer list sorted by deadline (earliest deadline first).
	list_for_each_entry(alarm_entry, alarm_t, &alarms, list_alarm) {
		if (alarm->deadline < alarm_entry->deadline) {
			list_add_tail(&alarm->list_alarm, &alarm_entry->list_alarm);
			return;
		}
	}
	list_add_tail(&alarm->list_alarm, &alarms);
	if (needs_reschedule || is_first_alarm(alarm))
		reschedule_root_alarm();
}

static void *callback_dispatch(void *context) {
	alarm_t *alarm;

	(void) context;
	while (true) {
		sema_wait(&alarm_expired);
		if (!dispatcher_thread_active)
			break;
		mutex_lock(&alarms_mutex);
		// Take into account that the alarm may get cancelled before we get to it.
		// We're done here if there are no alarms or the alarm at the front is in
		// the future. Exit right away since there's nothing left to do.
		if (list_empty(&alarms) ||
			(alarm = list_first_entry(&alarms, alarm_t, list_alarm))->deadline >
			now()) {
			reschedule_root_alarm();
			mutex_unlock(&alarms_mutex);
			continue;
		}
		list_del_init(&alarm->list_alarm);
		if (alarm->is_periodic) {
//			alarm->prev_deadline = alarm->deadline;// TODO ?
			schedule_next_instance(alarm);
//			alarm->stats.rescheduled_count++;//TODO ?
		}
		reschedule_root_alarm();
		// Enqueue the alarm for processing
		blocking_queue_push(alarm->queue, alarm);
		mutex_unlock(&alarms_mutex);
		//TODO what happened to alarm if not periodic ????
	}
	LOG_DEBUG("Callback thread exited");
	return NULL;
}

#ifdef HAS_TIMER
static void timer_callback(void *ptr) {
	(void) ptr;
	sema_post(&alarm_expired);
}

static bool timer_create_internal(const clockid_t clock_id, timer_t *timer) {
	struct sigevent sigevent;

	// create timer with RT priority thread
	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
	struct sched_param param;
	param.sched_priority = THREAD_RT_PRIORITY;
	pthread_attr_setschedparam(&thread_attr, &param);
	bzero(&sigevent, sizeof(sigevent));
	sigevent.sigev_notify = SIGEV_THREAD;
	sigevent.sigev_notify_function = (void (*)(union sigval)) timer_callback;
	sigevent.sigev_notify_attributes = &thread_attr;
	if (timer_create(clock_id, &sigevent, timer) == -1) {
		LOG_ERROR("Unable to create timer with clock %d: %m", clock_id);
		if (clock_id == CLOCK_BOOTTIME_ALARM) {
			LOG_ERROR("The kernel might not have support for "
					  "timer_create(CLOCK_BOOTTIME_ALARM): "
					  "https://lwn.net/Articles/429925/");
			LOG_ERROR("See following patches: "
					  "https://git.kernel.org/cgit/linux/kernel/git/torvalds/"
					  "linux.git/log/?qt=grep&q=CLOCK_BOOTTIME_ALARM");
		}
		return false;
	}
	return true;
}

#endif

static void alarm_queue_ready(blocking_queue_t *queue) {
	alarm_t *alarm;
	void *data;

	mutex_lock(&alarms_mutex);
	alarm = (alarm_t *) blocking_queue_trypop(queue);
	if (alarm == NULL) { // The alarm was probably canceled
		mutex_unlock(&alarms_mutex);
		return;
	}

	// If the alarm is not periodic, we've fully serviced it now, and can reset
	// some of its internal state. This is useful to distinguish between expired
	// alarms and active ones.
	work_t *callback = alarm->callback;
	data = alarm->data;
	if (!alarm->is_periodic) {
		alarm->deadline = 0;
		alarm->callback = NULL;
		alarm->data = NULL;
	}
	mutex_lock(&alarm->callback_mutex);
	mutex_unlock(&alarms_mutex);
	callback(data);
	mutex_unlock(&alarm->callback_mutex);
	//TODO if not periodic do we need to free it ?
}

static bool alarm_lazy_init(void) {
	if (is_ready)
		return (true);
	if (!init_started) {
		init_started = true;
		mutex_init(&alarms_mutex);
	}
	mutex_lock(&alarms_mutex);
	if (is_ready) {
		mutex_unlock(&alarms_mutex);
		return (true);
	}
#ifdef HAS_TIMER

	if (!timer_create_internal(CLOCK_ID, &timer))
		goto error;
	if (!timer_create_internal(CLOCK_ID_ALARM, &wakeup_timer))
		goto error1;
#endif
	if (sema_init(&alarm_expired, 0))
		goto error2;
	if (thread_init(&default_callback_thread, "alarm_default_callback"))
		goto error3;
	//TODO priority here ?
	if (blocking_queue_init(&default_callback_queue, UINT32_MAX))
		goto error4;
	blocking_queue_listen(&default_callback_queue, &default_callback_thread,
						  alarm_queue_ready);
	if (thread_init(&dispatcher_thread, "alarm_dispatcher"))
		goto error5;
	dispatcher_thread_active = true;
	if (!thread_post(&dispatcher_thread, callback_dispatch, NULL))
		goto error6;
	is_ready = true;
	mutex_unlock(&alarms_mutex);
	return true;
	error6:
	dispatcher_thread_active = false;
	thread_destroy(&dispatcher_thread);
	error5:
	blocking_queue_destroy(&default_callback_queue, NULL);
	error4:
	thread_destroy(&default_callback_thread);
	error3:
	sema_destroy(&alarm_expired);
	error2:
#ifdef HAS_TIMER
	timer_delete(wakeup_timer);
	error1:
	timer_delete(timer);
	error:
#endif
	mutex_unlock(&alarms_mutex);
	return false;
}

static void alarm_cancel_internal(alarm_t *alarm) {
	// Check if alarm is first entry of alarms
	const bool needs_reschedule = is_first_alarm(alarm);
	remove_pending_alarm(alarm);
	alarm->deadline = 0;
	alarm->callback = NULL;
	alarm->data = NULL;
	alarm->queue = NULL;
	if (needs_reschedule)
		reschedule_root_alarm();
}

void alarm_cancel(alarm_t *alarm) {
	if (!alarm)
		return;
	mutex_lock(&alarms_mutex);
	alarm_cancel_internal(alarm);
	mutex_unlock(&alarms_mutex);
	// If the callback for |alarm| is in progress, wait here until it completes.
	mutex_lock(&alarm->callback_mutex);
	mutex_unlock(&alarm->callback_mutex);
}

static bool alarm_init_internal(alarm_t *alarm,
								const char *name,
								bool is_periodic) {
	bzero(alarm, sizeof(alarm_t));
	alarm->is_periodic = is_periodic;
	alarm->name = strdup(name);
	mutex_init(&alarm->callback_mutex);
	//TODO this mutex was recursive. Is it ok ?
	//TODO
	return true;
}

bool alarm_init(alarm_t *alarm, const char *name) {
	if (!alarm_lazy_init())
		return false;
	return alarm_init_internal(alarm, name, false);
}

bool alarm_init_periodic(alarm_t *alarm, const char *name) {
	if (!alarm_lazy_init())
		return false;
	return alarm_init_internal(alarm, name, false);
}

void alarm_destroy(alarm_t *alarm) {
	alarm_cancel(alarm);
	mutex_destroy(&alarm->callback_mutex);
}

static alarm_t *alarm_new_internal(const char *name, bool is_periodic) {
	alarm_t *new_alarm;

	if (!alarm_lazy_init())
		return NULL;
	if (!(new_alarm = malloc(sizeof(new_alarm))))
		return NULL;
	if (!(alarm_init_internal(new_alarm, name, is_periodic))) {
		free(new_alarm);
		return NULL;
	}
	return new_alarm;
}

void alarm_free(alarm_t *alarm) {
	if (!alarm)
		return;
	alarm_destroy(alarm);
	free((void *) alarm->name);
	free(alarm);
}

alarm_t *alarm_new(const char *name) {
	return alarm_new_internal(name, false);
}

alarm_t *alarm_new_periodic(const char *name) {
	return alarm_new_internal(name, true);
}

// Runs in exclusion with alarm_cancel and timer_callback.
static void alarm_set_internal(alarm_t *alarm, period_ms_t period,
							   work_t cb, void *data,
							   blocking_queue_t *queue) {
//	assert(alarm != NULL);
//	assert(cb != NULL);
	mutex_lock(&alarms_mutex);
	alarm->creation_time = now();
	alarm->period = period;
	alarm->queue = queue;
	alarm->callback = cb;
	alarm->data = data;
	schedule_next_instance(alarm);
//	alarm->stats.scheduled_count++;
	mutex_unlock(&alarms_mutex);
}

void alarm_set_on_queue(alarm_t *alarm, period_ms_t interval_ms,
						work_t cb, void *data,
						blocking_queue_t *queue) {
//	assert(queue != NULL);
	alarm_set_internal(alarm, interval_ms, cb, data, queue);
}

void alarm_set(alarm_t *alarm, period_ms_t interval_ms,
			   work_t cb, void *data) {
	alarm_set_on_queue(alarm, interval_ms, cb, data, &default_callback_queue);
}


bool alarm_is_scheduled(const alarm_t *alarm) {
	if (alarm == NULL)
		return false;
	return (alarm->callback != NULL);
}