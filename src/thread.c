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

#define LOG_TAG "osi_thread"

#include "osi/log.h"
#include "osi/thread.h"
#include "osi/sema.h"
#include "osi/string.h"

#define DEFAULT_WORK_QUEUE_CAPACITY 128

typedef struct {
	thread_t *thread;
	sema_t start_sem;
	int error;
} start_arg_t;

typedef struct {
	work_t *func;
	void *context;
} work_item_t;

static void __work_ready(blocking_queue_t *queue)
{
	work_item_t *item;

	item = blocking_queue_pop(queue);
	item->func(item->context);
	free(item);
}

static void *__run_thread(void *context)
{
	start_arg_t *arg;
	thread_t *thread;
	work_item_t *item;
	unsigned count;

	arg = (start_arg_t *)context;
	thread = arg->thread;

	blocking_queue_listen(&thread->work_queue, thread, __work_ready);
	sema_post(&arg->start_sem);
	LOG_INFO("thread name %s started", thread->name);
	reactor_start(&thread->reactor);
	blocking_queue_unlisten(&thread->work_queue);

	/*
	 * Make sure we dispatch all queued work items before exiting the thread.
     * This allows a caller to safely tear down by enqueuing a teardown
     * work item and then joining the thread.
	 */
	count = 0;
	while ((item = blocking_queue_trypop(&thread->work_queue))
		   && count <= blocking_queue_capacity(&thread->work_queue)) {
		item->func(item->context);
		free(item);
		++count;
	}

	if (count > blocking_queue_capacity(&thread->work_queue))
		LOG_DEBUG("Growing event queue on shutdown.");

	LOG_INFO("thread name %s exited", thread->name);
	return NULL;
}

int thread_init(thread_t *thread, char const *name)
{
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	start.thread = thread;
	start.error = 0;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	blocking_queue_init(&thread->work_queue, DEFAULT_WORK_QUEUE_CAPACITY);
	reactor_init(&thread->reactor);
	task_spawn(&thread->task, __run_thread, &start);
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	if (start.error) {
		blocking_queue_destroy(&thread->work_queue, free);
		reactor_destroy(&thread->reactor);
	}
	return 0;
}

void thread_destroy(thread_t *thread)
{
	thread_stop(thread);
	thread_join(thread);
	task_destroy(&thread->task);
	blocking_queue_destroy(&thread->work_queue, free);
	reactor_destroy(&thread->reactor);
}

bool thread_setpriority(thread_t *thread, int priority)
{
	return task_setpriority(&thread->task, priority) == 0;
}

void thread_join(thread_t *thread)
{
	task_join(&thread->task);
}

bool thread_post(thread_t *thread, work_t *work, void *context)
{
	work_item_t *item;

	if (!(item = (work_item_t *)malloc(sizeof(work_item_t))))
		return false;
	item->func = work;
	item->context = context;
	blocking_queue_push(&thread->work_queue, item);
	return true;
}

void thread_stop(thread_t *thread)
{
	reactor_stop(&thread->reactor);
}
