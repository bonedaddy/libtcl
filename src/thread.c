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
	int fd;
	start_arg_t *arg;
	thread_t *thread;
	reactor_object_t *reactor_object;
	head_t *head;
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
	LOG_INFO("thread name %s started", thread->name);
	sema_post(&arg->start_sem);

	fd = thread->work_queue.dequeue_sem.handle;
	if (!(reactor_object = reactor_register(
		&thread->reactor, fd, &thread->work_queue,
		__work_ready, NULL))) {
		LOG_ERROR("unable to register reactor: %m");
		arg->error = errno;
		sema_post(&arg->start_sem);
		return NULL;
	}
	reactor_start(&thread->reactor);
	reactor_unregister(reactor_object);

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
		++count;
	}

	if (count > equeue_capacity(&thread->work_queue))
		LOG_DEBUG("Growing event queue on shutdown.");

	LOG_INFO("thread name %s exited", thread->name);
	return NULL;
}

static void __work_dtor(head_t *head)
{
	work_item_t *work;

	work = LIST_ENTRY(head, work_item_t, hold);
	free(work);
}
#endif /* OSI_THREADING */

int thread_init(thread_t *thread, char const *name)
{
#ifdef OSI_THREADING
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	thread->is_joined = false;
	start.thread = thread;
	start.error = 0;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	reactor_init(&thread->reactor);
	equeue_init(&thread->work_queue, DEFAULT_WORK_QUEUE_CAPACITY);
	pthread_create(&thread->pthread, NULL, __run_thread, &start);
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	if (start.error) {
		equeue_destroy(&thread->work_queue, __work_dtor);
		reactor_destroy(&thread->reactor);
	}
#else
	strncpy(thread->name, name, THREAD_NAME_MAX);
	list_init(&thread->fibers);
#endif /* OSI_THREADING */
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
	/* TODO */
#endif /* OSI_THREADING */
}

void thread_join(thread_t *thread)
{
#ifdef OSI_THREADING
	if (!thread->is_joined) {
		thread->is_joined = true;
		pthread_join(thread->pthread, NULL);
	}
#else
	head_t *head;
	fiber_t *fiber;

	while ((head = list_pop(&thread->fibers))) {
		fiber = LIST_ENTRY(head, fiber_t, hold);
		while (!fiber_isdone(fiber))
			fiber_call(fiber, NULL);
	}
#endif /* OSI_THREADING */
}

bool thread_post(thread_t *thread, work_t *work, void *context)
{
#ifdef OSI_THREADING
	work_item_t *item;

	item = (work_item_t *)malloc(sizeof(work_item_t));
	item->func = work;
	item->context = context;
	head_init(&item->hold);
	equeue_push(&thread->work_queue, &item->hold);
#else
	(void)thread;
	(void)work;
	(void)context;
#endif /* OSI_THREADING */
	return true;
}

void thread_stop(thread_t *thread)
{
#ifdef OSI_THREADING
	reactor_stop(&thread->reactor);
#else
	(void)thread;
#endif /* OSI_THREADING */
}
