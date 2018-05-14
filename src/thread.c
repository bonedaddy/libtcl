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

#ifdef OSI_THREAD_MOD
typedef struct start_arg {
	thread_t *thread;
	sema_t start_sem;
	int error;
} start_arg_t;

static void *__run_thread(void *start_arg);
#endif /* OSI_THREAD_MOD */

int thread_init(thread_t *thread, char const *name)
{
#ifdef OSI_THREAD_MOD
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	thread->tid = 0;
	thread->is_joined = false;
	start.thread = thread;
	start.error = 0;
	strncpy(thread->name, name, THREAD_NAME_MAX);
	reactor_init(&thread->reactor);
	equeue_init(&thread->work_queue);
	pthread_create(&thread->pthread, NULL, __run_thread, &start);
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	if (start.error) {
		equeue_destroy(&thread->work_queue);
		reactor_destroy(&thread->reactor);
	}
#else
	strncpy(thread->name, name, THREAD_NAME_MAX);
	list_init(&thread->fibers);
#endif /* OSI_THREAD_MOD */
	return 0;
}

void thread_destroy(thread_t *thread)
{
	thread_stop(thread);
	thread_join(thread);
#ifdef OSI_THREAD_MOD
	equeue_destroy(&thread->work_queue);
	reactor_destroy(&thread->reactor);
#else
	list_init(&thread->fibers);
#endif /* OSI_THREAD_MOD */
}

void thread_join(thread_t *thread)
{
#ifdef OSI_THREAD_MOD
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
#endif /* OSI_THREAD_MOD */
}

bool thread_post(thread_t *thread, work_t *work, void *context)
{
#ifdef OSI_THREAD_MOD
#else
#endif /* OSI_THREAD_MOD */
}

void thread_stop(thread_t *thread)
{
#ifdef OSI_THREAD_MOD
#else
#endif /* OSI_THREAD_MOD */
}

bool thread_priority(thread_t *thread, int priority)
{
#ifdef OSI_THREAD_MOD
#else
#endif /* OSI_THREAD_MOD */
}
