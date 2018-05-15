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

#include <osi/log.h>
#include <osi/thread.h>
#include <osi/sema.h>
#include <osi/string.h>

#define DEFAULT_WORK_QUEUE_CAPACITY 128

typedef struct {
	thread_t *thread;
	sema_t start_sem;
	int error;
} start_arg_t;

typedef struct {
	work_t *func;
	void *context;
	head_t hold;
} work_item_t;

static void __work_ready(void *context)
{
	equeue_t *queue;
	head_t *head;
	work_item_t *item;

	queue = (equeue_t *)context;
	head = equeue_pop(queue);
	item = LIST_ENTRY(head, work_item_t, hold);
	item->func(item->context);
	free(item);
}


static void *__run_thread(void *context)
{
	start_arg_t *arg;
	thread_t *thread;
	head_t *head;
	work_item_t *item;
	unsigned count;
#ifdef OSI_THREADING
	int fd;
	reactor_object_t *reactor_object;
#endif

	arg = (start_arg_t *)context;
	thread = arg->thread;

#ifdef OSI_THREADING
	if (prctl(PR_SET_NAME, (unsigned long)thread->name) == -1) {
		LOG_ERROR("unable to set thread name: %m");
		arg->error = errno;
		sema_post(&arg->start_sem);
		return NULL;
	}
#endif
	sema_post(&arg->start_sem);
	LOG_INFO(LOG_TAG, "thread name %s started", thread->name);


#ifdef OSI_THREADING
	fd = thread->work_queue.dequeue_sem.handle;
	if (!(reactor_object = reactor_register(
		&thread->reactor, fd, &thread->work_queue, __work_ready, NULL))) {
		LOG_ERROR("unable to register reactor: %m");
		arg->error = errno;
		sema_post(&arg->start_sem);
		return NULL;
	}
	reactor_start(&thread->reactor);
	reactor_unregister(reactor_object);
#else
	while (thread->fiber.status == OSI_FIB_RUNNING) {
		if ((head = equeue_trypop(&thread->work_queue))) {
			item = LIST_ENTRY(head, work_item_t, hold);
			item->func(item->context);
			free(item);
		}
		fiber_yield(NULL);
	}
#endif

	/*
	 * Make sure we dispatch all queued work items before exiting the thread.
     * This allows a caller to safely tear down by enqueuing a teardown
     * work item and then joining the thread.
	 */
	count = 0;
	while ((head = equeue_trypop(&thread->work_queue))
		&& count <= equeue_capacity(&thread->work_queue)) {
		item = LIST_ENTRY(head, work_item_t, hold);
		item->func(item->context);
		free(item);
		++count;
	}

	if (count > equeue_capacity(&thread->work_queue))
		LOG_DEBUG(LOG_TAG, "Growing event queue on shutdown.");

	LOG_INFO(LOG_TAG, "thread name %s exited", thread->name);
	return NULL;
}

static void __work_dtor(head_t *head)
{
	work_item_t *work;

	work = LIST_ENTRY(head, work_item_t, hold);
	free(work);
}


int thread_init(thread_t *thread, char const *name)
{
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	thread->is_joined = false;
	start.thread = thread;
	start.error = 0;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	equeue_init(&thread->work_queue, DEFAULT_WORK_QUEUE_CAPACITY);
#ifdef OSI_THREADING
	reactor_init(&thread->reactor);
	pthread_create(&thread->pthread, NULL, __run_thread, &start);
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
#else
	fiber_init(&thread->fiber, __run_thread, 4096, FIBER_NONE);
	fiber_call(&thread->fiber, &start);
#endif /* OSI_THREADING */
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	if (start.error) {
		equeue_destroy(&thread->work_queue, __work_dtor);
#ifdef OSI_THREADING
		reactor_destroy(&thread->reactor);
#endif /* OSI_THREADING */
	}
	return 0;
}

void thread_destroy(thread_t *thread)
{
	thread_stop(thread);
	thread_join(thread);
#ifdef OSI_THREADING
	equeue_destroy(&thread->work_queue, __work_dtor);
	reactor_destroy(&thread->reactor);
#else
	fiber_destroy(&thread->fiber);
#endif /* OSI_THREADING */
}

void thread_join(thread_t *thread)
{
	if (!thread->is_joined) {
		thread->is_joined = true;
#ifdef OSI_THREADING
		pthread_join(thread->pthread, NULL);
#else
		while (!fiber_isdone(&thread->fiber))
			fiber_call(&thread->fiber, NULL);
#endif /* OSI_THREADING */
	}
}

bool thread_post(thread_t *thread, work_t *work, void *context)
{
	work_item_t *item;

	item = (work_item_t *)malloc(sizeof(work_item_t));
	item->func = work;
	item->context = context;
	head_init(&item->hold);
	equeue_push(&thread->work_queue, &item->hold);
#ifndef OSI_THREADING
	fiber_call(&thread->fiber, NULL);
#endif /* !OSI_THREADING */
	return true;
}

void thread_stop(thread_t *thread)
{
#ifdef OSI_THREADING
	reactor_stop(&thread->reactor);
#else
	thread->fiber.status = OSI_FIB_EXITING;
#endif /* OSI_THREADING */
}
