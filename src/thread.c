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

#include <osi/thread.h>
#include <osi/sema.h>

#ifdef OSI_THREADING
typedef struct {
	thread_t *thread;
	sema_t start_sem;
	int error;
} start_arg_t;

typedef struct {
	work_t *func;
	void *context;
	node_t hold;
} work_item_t;

static void *__run_thread(void *start_arg)
{
	(void)start_arg;
	return NULL;
}

static void __work_dtor(node_t *head)
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
	thread->tid = 0;
	thread->is_joined = false;
	start.thread = thread;
	start.error = 0;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	reactor_init(&thread->reactor);
	equeue_init(&thread->work_queue, 128);
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
	node_t *head;
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
	(void)thread;
	(void)work;
	(void)context;
#ifdef OSI_THREADING
#else
#endif /* OSI_THREADING */
	return true;
}

void thread_stop(thread_t *thread)
{
	(void)thread;
#ifdef OSI_THREADING
#else
#endif /* OSI_THREADING */
}

bool thread_priority(thread_t *thread, int priority)
{
	(void)thread;
	(void)priority;
#ifdef OSI_THREADING
#else
#endif /* OSI_THREADING */
	return true;
}
