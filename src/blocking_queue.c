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

#include <osi/blocking_queue.h>
#include <osi/thread.h>
#include <osi/string.h>

int blocking_queue_init(blocking_queue_t *blocking_queue, unsigned capacity)
{
	bzero(blocking_queue, sizeof(blocking_queue_t));
	blocking_queue->capacity = capacity;
	if (sema_init(&blocking_queue->enqueue_sem, capacity))
		return -1;
	if (sema_init(&blocking_queue->dequeue_sem, 0))
		return -1;
	list_init(&blocking_queue->list);
#ifdef OSI_THREADING
	pthread_mutex_init(&blocking_queue->lock, NULL);
#endif
	return 0;
}

void blocking_queue_destroy(blocking_queue_t *blocking_queue, head_dtor_t *dtor)
{
	blocking_queue_unlisten(blocking_queue);
	list_destroy(&blocking_queue->list, dtor);
	sema_destroy(&blocking_queue->enqueue_sem);
	sema_destroy(&blocking_queue->dequeue_sem);
#ifdef OSI_THREADING
	pthread_mutex_destroy(&blocking_queue->lock);
#endif
}

bool blocking_queue_empty(blocking_queue_t *blocking_queue)
{
	bool empty;

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	empty = list_empty(&blocking_queue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#endif
	return empty;
}

unsigned blocking_queue_length(blocking_queue_t *blocking_queue)
{
	unsigned len;

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	len = (unsigned)blocking_queue->list.len;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#endif
	return len;
}

unsigned blocking_queue_capacity(blocking_queue_t *blocking_queue)
{
	return blocking_queue->capacity;
}

void blocking_queue_push(blocking_queue_t *blocking_queue, head_t *ev)
{
	sema_wait(&blocking_queue->enqueue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	list_push(&blocking_queue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#else
	if (blocking_queue->listener) {
		sched_spawn(stdsched, (work_t *)blocking_queue->listener, 32, blocking_queue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&blocking_queue->dequeue_sem);
}

head_t *blocking_queue_pop(blocking_queue_t *blocking_queue)
{
	head_t *ev;

	sema_wait(&blocking_queue->dequeue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	ev = list_shift(&blocking_queue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#endif
	sema_post(&blocking_queue->enqueue_sem);
	return ev;
}

bool blocking_queue_trypush(blocking_queue_t *blocking_queue, head_t *ev)
{
	if (!sema_trywait(&blocking_queue->enqueue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	list_push(&blocking_queue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#else
	if (blocking_queue->listener) {
		sched_spawn(stdsched, (work_t *)blocking_queue->listener, 32, blocking_queue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&blocking_queue->dequeue_sem);
	return true;
}

head_t *blocking_queue_trypop(blocking_queue_t *blocking_queue)
{
	head_t *ev;

	if (!sema_trywait(&blocking_queue->dequeue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&blocking_queue->lock);
#endif
	ev = list_shift(&blocking_queue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&blocking_queue->lock);
#endif
	sema_post(&blocking_queue->enqueue_sem);
	return ev;
}

void blocking_queue_listen(blocking_queue_t *blocking_queue, thread_t *thread, listener_t *listener)
{
	blocking_queue->listener = listener;
#ifdef OSI_THREADING
	blocking_queue_unlisten(blocking_queue);
	blocking_queue->reactor_object = reactor_register(
		&thread->reactor, blocking_queue->dequeue_sem.handle, blocking_queue,
		(reactor_ready_t *)listener, NULL);
#else
	blocking_queue->thread = thread;
#endif /* OSI_THREADING */
}

void blocking_queue_unlisten(blocking_queue_t *blocking_queue)
{
#ifdef OSI_THREADING
	if (blocking_queue->reactor_object) {
		reactor_unregister(blocking_queue->reactor_object);
		blocking_queue->reactor_object = NULL;
	}
#else
	blocking_queue->listener = NULL;
#endif /* OSI_THREADING */
}
