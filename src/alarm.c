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

#include "osi/alarm.h"
#include "osi/list.h"
#include "osi/log.h"
#include "osi/mutex.h"
#include "osi/task.h"
#include "osi/thread.h"

#define THREAD_RT_PRIORITY 1

static bool init_started = false;
static bool is_ready = false; /*TODO: could replace __dispatcher_active */
static LIST_HEAD(alarms);
static mutex_t alarms_mutex;
static bool __dispatcher_active = false;
static task_t __dispatcher;
static thread_t default_callback_thread;
static blocking_queue_t default_callback_queue;
static sema_t alarm_expired;

#ifdef HAS_TIMER
static timer_t timer = NULL;
static timer_t wakeup_timer = NULL;
static bool timer_set = false;

static bool __timer_init(const clockid_t clock_id, timer_t *timer)
{
	struct sigevent sigevent;
	pthread_attr_t thread_attr;
	struct sched_param param;

	pthread_attr_init(&thread_attr);
	pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
	param.sched_priority = THREAD_RT_PRIORITY;
	pthread_attr_setschedparam(&thread_attr, &param);
	bzero(&sigevent, sizeof(sigevent));
	sigevent.sigev_notify = SIGEV_THREAD;
	sigevent.sigev_notify_function = (void (*)(union sigval))sema_post;
	sigevent.sigev_notify_attributes = &thread_attr;
	sigevent.sigev_value.sival_ptr = &alarm_expired;
	if (timer_create(clock_id, &sigevent, timer) == -1) {
		LOG_ERROR("Unable to create timer with clock %d: %m", clock_id);
		if (clock_id == CLOCK_BOOTTIME_ALARM) {
			LOG_ERROR("The kernel might not have support for "
				"timer_create(CLOCK_BOOTTIME_ALARM): "
				"https://lwn.net/Articles/429925/"
				"See following patches: "
				"https://git.kernel.org/cgit/linux/kernel/git/torvalds/"
				"linux.git/log/?qt=grep&q=CLOCK_BOOTTIME_ALARM");
		}
		return false;
	}
	return true;
}
#endif

static __always_inline bool __schedule_needed(alarm_t *alarm)
{
	return !list_empty(&alarms) &&
		list_first_entry(&alarms, alarm_t, list_alarm) == alarm;
}

static void __unschedule(alarm_t *alarm)
{
	list_del_init(&alarm->list_alarm);
	if (blocking_queue_remove(alarm->queue, alarm)) {
		/* We shouldn't have repeated alarm instances */
		assert(blocking_queue_remove(alarm->queue, alarm) == false);
	}
}

static void __schedule_root(void)
{
#ifdef HAS_TIMER
# ifdef HAS_WAKELOCK
	const bool timer_was_set = timer_set;
# endif
	/* If used in a zeroed state, disarms the timer. */
	struct itimerspec timer_time;
	struct itimerspec end_of_time;
	struct itimerspec wakeup_time;
	struct itimerspec time_to_expire;
	period_ms_t next_deadline;
	int64_t next_expiration;

	bzero(&timer_time, sizeof(struct itimerspec));
	if (list_empty(&alarms))
		goto done;

	next_deadline = list_first_entry(&alarms, alarm_t, list_alarm)->deadline;
	next_expiration = next_deadline - now();
	if (next_expiration < TIMER_INTERVAL_FOR_WAKELOCK_IN_MS) {
# ifdef HAS_WAKELOCK
		if (!timer_set) {
			if (!wakelock_acquire()) {
				LOG_ERROR(LOG_TAG, "%s unable to acquire wake lock", __func__);
				goto done;
			}
		}
# endif
		timer_time.it_value.tv_sec = (next_deadline / 1000);
		timer_time.it_value.tv_nsec = (next_deadline % 1000) * 1000000LL;
		bzero(&end_of_time, sizeof(end_of_time));
		end_of_time.it_value.tv_sec = (time_t)(1LL << (sizeof(time_t) * 8 - 2));
		if (timer_settime(wakeup_timer, TIMER_ABSTIME, &end_of_time, NULL) < 0)
			LOG_ERROR("Unable to set wakeup timer: %m");
	} else {
		bzero(&wakeup_time, sizeof(wakeup_time));
		wakeup_time.it_value.tv_sec = (next_deadline / 1000);
		wakeup_time.it_value.tv_nsec = (next_deadline % 1000) * 1000000LL;
		if (timer_settime(wakeup_timer, TIMER_ABSTIME, &wakeup_time, NULL) < 0)
			LOG_ERROR("Unable to set wakeup timer: %m");
	}
done:
	timer_set = timer_time.it_value.tv_sec || timer_time.it_value.tv_nsec;
# ifdef HAS_WAKELOCK
	if (timer_was_set && !timer_set) {
		wakelock_release();
	}
# endif
	if (timer_settime(timer, TIMER_ABSTIME, &timer_time, NULL) == -1)
		LOG_ERROR("Unable to set timer: %m");
	if (timer_set) {
		timer_gettime(timer, &time_to_expire);
		if (!time_to_expire.it_value.tv_sec &&
			!time_to_expire.it_value.tv_nsec) {
			LOG_DEBUG("Alarm expiration too close for posix timers, "
				"switching to guns");
			sema_post(&alarm_expired);
		}
	}
#endif
}

static void __schedule(alarm_t *alarm)
{
	period_ms_t just_now;
	period_ms_t ms_into_period;
	alarm_t *alarm_entry;

	if (alarm->callback) __unschedule(alarm);
	ms_into_period = 0;
	just_now = now();
	if (alarm->is_periodic && alarm->period != 0)
		ms_into_period = ((just_now - alarm->creation_time) % alarm->period);
	alarm->deadline = just_now + (alarm->period - ms_into_period);
	list_for_each_entry(alarm_entry, alarm_t, &alarms, list_alarm) {
		if (alarm->deadline < alarm_entry->deadline) {
			list_add_tail(&alarm->list_alarm, &alarm_entry->list_alarm);
			return ;
		}
	}
	list_add_tail(&alarm->list_alarm, &alarms);
}

static void *__schedule_loop(void *context)
{
	alarm_t *alarm;

	(void)context;
#ifdef HAS_TIMER
	sema_wait(&alarm_expired);
#endif
	if (!__dispatcher_active) {
		task_stop(&__dispatcher);
		return NULL;
	}
	mutex_lock(&alarms_mutex);
	if (list_empty(&alarms) ||
		(alarm = list_first_entry(&alarms, alarm_t, list_alarm))->deadline >
		now()) {
		__schedule_root();
		mutex_unlock(&alarms_mutex);
		return NULL;
	}
	list_del_init(&alarm->list_alarm);
	if (alarm->is_periodic)
		__schedule(alarm);
	__schedule_root();
	if (alarm->callback)
		blocking_queue_push(alarm->queue, alarm);
	mutex_unlock(&alarms_mutex);
	return NULL;
}

static void __dispatch_ready(blocking_queue_t *queue)
{
	alarm_t *alarm;
	proc_t *callback;
	void *data;

	if (!__dispatcher_active)
		return ;
	mutex_lock(&alarms_mutex);
	if (!(alarm = (alarm_t *)blocking_queue_trypop(queue)) ||
		!alarm->callback) {
		/* Alarm was canceled, return */
		mutex_unlock(&alarms_mutex);
		return;
	}

	/* If the alarm is not periodic, we've fully serviced it now, and can reset
	 * some of its internal state. This is useful to distinguish between expired
	 * alarms and active ones.
	 */
	callback = alarm->callback;
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
}

static bool __lazyinit(void)
{
	if (is_ready) return (true);
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
	if (!__timer_init(CLOCK_ID, &timer))
		goto error;
	if (!__timer_init(CLOCK_ID_ALARM, &wakeup_timer))
		goto error1;
#endif
	if (sema_init(&alarm_expired, 0))
		goto error2;
	if (thread_init(&default_callback_thread, "alarm_default_callback"))
		goto error3;
	//TODO priority here ?
	if (blocking_queue_init(&default_callback_queue, UINT32_MAX))
		goto error4;
	alarm_register(&default_callback_queue,
		&default_callback_thread);
	__dispatcher_active = true;
	if (task_repeat(&__dispatcher, __schedule_loop, NULL))
		goto error5;
	is_ready = true;
	mutex_unlock(&alarms_mutex);
	return true;
error5:
	__dispatcher_active = false;
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

static __always_inline int __init(alarm_t *alarm, const char *name,
	bool is_periodic)
{
	if (!__lazyinit())
		return -1;

	bzero(alarm, sizeof(alarm_t));
	strncpy(alarm->name, name, ALARM_NAME_MAX - 1);
	if (mutex_init(&alarm->callback_mutex))
		return -1;
	INIT_LIST_HEAD(&alarm->list_alarm);
	alarm->is_periodic = is_periodic;
	alarm->is_set = false;
	return 0;
}

static void __cancel(alarm_t *alarm)
{
	const bool needs_reschedule = __schedule_needed(alarm);

	alarm->callback = NULL;
	__unschedule(alarm);
	alarm->deadline = 0;
	alarm->data = NULL;
	alarm->queue = NULL;
	alarm->is_set = false;
	if (needs_reschedule) __schedule_root();
}

int alarm_init(alarm_t *alarm, const char *name)
{
	return __init(alarm, name, false);
}

int alarm_init_periodic(alarm_t *alarm, const char *name)
{
	return __init(alarm, name, true);
}

__always_inline void alarm_destroy(alarm_t *alarm)
{
	alarm_cancel(alarm);
	mutex_destroy(&alarm->callback_mutex);
}

__always_inline bool alarm_is_scheduled(const alarm_t *alarm)
{
	return (alarm && alarm->callback);
}

__always_inline void alarm_cancel(alarm_t *alarm)
{
	mutex_lock(&alarms_mutex);
	__cancel(alarm);
	mutex_unlock(&alarms_mutex);
}

void alarm_attach(alarm_t *alarm, period_ms_t period, proc_t *cb, void *data,
	blocking_queue_t *queue)
{
	assert(!alarm->is_set);
	/* please uael, don't remove this..*/
	// TODO fix this shit
	// TODO 2, dont
	assert(!alarm->is_periodic || period >= 25);

	mutex_lock(&alarms_mutex);
	alarm->creation_time = now();
	alarm->period = period;
	alarm->queue = queue;
	alarm->callback = cb;
	alarm->data = data;
	__schedule(alarm);
	alarm->is_set = true;
	if (__schedule_needed(alarm))
		__schedule_root();
	mutex_unlock(&alarms_mutex);
}

__always_inline void alarm_set(alarm_t *alarm, period_ms_t period,
	proc_t *cb, void *data)
{
	alarm_attach(alarm, period, cb, data, &default_callback_queue);
}

__always_inline void alarm_register(blocking_queue_t *queue, thread_t *thread)
{
	blocking_queue_listen(queue, thread, __dispatch_ready);
}

void alarm_unregister(blocking_queue_t *queue)
{
	alarm_t *alarm_entry;
	alarm_t *tmp;

	blocking_queue_unlisten(queue);

	/* Cancel all alarms that are using this queue */
	mutex_lock(&alarms_mutex);
	list_for_each_entry_safe(alarm_entry, tmp, alarm_t, &alarms, list_alarm) {
		if (alarm_entry->queue == queue)
			__cancel(alarm_entry);
	}
	mutex_unlock(&alarms_mutex);
}

void alarm_cleanup(void)
{
	alarm_t *alarm_entry;
	alarm_t *tmp;
	
	if (!is_ready)
		return;
	is_ready = false;
	__dispatcher_active = false;

	sema_post(&alarm_expired);
	task_destroy(&__dispatcher);
	mutex_lock(&alarms_mutex);
	blocking_queue_destroy(&default_callback_queue, NULL);
	thread_destroy(&default_callback_thread);
#ifdef HAS_TIMER
	if (timer_delete(wakeup_timer))
		LOG_ERROR("timer_delete wakeup_timer %m");
	wakeup_timer = NULL;
	if (timer_delete(timer))
		LOG_ERROR("timer_delete timer %m");
	timer = NULL;
#endif
	sema_destroy(&alarm_expired);
	list_for_each_entry_safe(alarm_entry, tmp, alarm_t, &alarms, list_alarm)
		alarm_destroy(alarm_entry);
	INIT_LIST_HEAD(&alarms);
	init_started = false;
	mutex_unlock(&alarms_mutex);
	mutex_destroy(&alarms_mutex);
}
