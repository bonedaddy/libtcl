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

#ifdef OSI_THREADING
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
	if (prctl(PR_SET_NAME, (unsigned long)thread->name) == -1) {
		LOG_ERROR("unable to set thread name: %m");
		arg->error = errno;
		sema_post(&arg->start_sem);
		return NULL;
	}
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
#endif

int thread_init(thread_t *thread, char const *name)
{
#ifdef OSI_THREADING
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	start.thread = thread;
	start.error = 0;
	thread->is_joined = false;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	blocking_queue_init(&thread->work_queue, DEFAULT_WORK_QUEUE_CAPACITY);
	reactor_init(&thread->reactor);
	pthread_create(&thread->pthread, NULL, __run_thread, &start);
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	if (start.error) {
		blocking_queue_destroy(&thread->work_queue, free);
		reactor_destroy(&thread->reactor);
	}
#else
	thread->is_joined = false;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	stack_init(&thread->fibers, sizeof(fid_t));
#endif /* OSI_THREADING */
	return 0;
}

void thread_destroy(thread_t *thread)
{
	thread_stop(thread);
	thread_join(thread);
#ifdef OSI_THREADING
	blocking_queue_destroy(&thread->work_queue, free);
	reactor_destroy(&thread->reactor);
#else
	stack_destroy(&thread->fibers, NULL);
#endif /* OSI_THREADING */
}

void thread_join(thread_t *thread)
{
#ifndef OSI_THREADING
	fid_t fid;

#endif
	if (!thread->is_joined) {
		thread->is_joined = true;
#ifdef OSI_THREADING
		pthread_join(thread->pthread, NULL);
#else
		while (stack_pop(&thread->fibers, &fid))
			fiber_join(fid);
		while (thread->running);
#endif /* OSI_THREADING */
	}
}

bool thread_post(thread_t *thread, work_t *work, void *context)
{
#ifdef OSI_THREADING
	work_item_t *item;

	if (!(item = (work_item_t *)malloc(sizeof(work_item_t))))
		return false;
	item->func = work;
	item->context = context;
	blocking_queue_push(&thread->work_queue, item);
#else
	fid_t fid;

	fiber_init(&fid, work, (fiber_attr_t){ });
	fiber_call(fid, context);
	*(fid_t *)stack_push(&thread->fibers) = fid;
#endif /* OSI_THREADING */
	return true;
}

void thread_stop(thread_t *thread)
{
#ifdef OSI_THREADING
	reactor_stop(&thread->reactor);
#else
	thread->running = false;
#endif /* OSI_THREADING */
}
