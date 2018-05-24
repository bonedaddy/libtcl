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

#include "osi/blocking_queue.h"
#include "osi/reactor.h"
#include "osi/thread.h"
#include "osi/string.h"

int blocking_queue_init(blocking_queue_t *queue, unsigned capacity)
{
	if (sema_init(&queue->producer, capacity))
		return -1;
	if (sema_init(&queue->consumer, 0))
		return -1;
	queue->capacity = capacity;
	queue_init(&queue->base, sizeof(void *));
	queue->reactor_event = NULL;
#ifdef OSI_THREADING
	pthread_mutex_init(&queue->lock, NULL);
#endif /* OSI_THREADING */
	return 0;
}

void blocking_queue_destroy(blocking_queue_t *queue, queue_dtor_t *dtor)
{
	blocking_queue_unlisten(queue);
	queue_destroy(&queue->base, dtor);
	sema_destroy(&queue->producer);
	sema_destroy(&queue->consumer);
#ifdef OSI_THREADING
	pthread_mutex_destroy(&queue->lock);
#endif /* OSI_THREADING */
}

bool blocking_queue_empty(blocking_queue_t *queue)
{
	bool empty;

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	empty = queue_length(&queue->base) == 0;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	return empty;
}

unsigned blocking_queue_length(blocking_queue_t *queue)
{
	unsigned len;

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	len = (unsigned)queue_length(&queue->base);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	return len;
}

unsigned blocking_queue_capacity(blocking_queue_t *queue)
{
	return queue->capacity;
}

void blocking_queue_push(blocking_queue_t *queue, void const *item)
{
	sema_wait(&queue->producer);
#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	*(void const **)queue_push(&queue->base) = item;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->consumer);
}

void *blocking_queue_pop(blocking_queue_t *queue)
{
	void *item;

	sema_wait(&queue->consumer);
#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	item = NULL;
	queue_pop(&queue->base, &item);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->producer);
	return item;
}

bool blocking_queue_trypush(blocking_queue_t *queue, const void *item)
{
	if (!sema_trywait(&queue->producer))
		return false;
#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	*(void const **)queue_push(&queue->base) = item;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->consumer);
	return true;
}

void *blocking_queue_trypop(blocking_queue_t *queue)
{
	void *item;

	if (!sema_trywait(&queue->consumer))
		return false;
#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	item = NULL;
	queue_pop(&queue->base, &item);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->producer);
	return item;
}

void blocking_queue_listen(blocking_queue_t *queue, thread_t *thread,
	listener_t *listener)
{
	blocking_queue_unlisten(queue);
	queue->reactor_event = reactor_register(
		&thread->reactor, &queue->consumer.event, queue,
		(reactor_ready_t *)listener, NULL);
}

void blocking_queue_unlisten(blocking_queue_t *queue)
{
	if (queue->reactor_event) {
		while (queue_length(&queue->base)
			&& queue->reactor_event->reactor->is_running)
			reactor_run_once(queue->reactor_event->reactor);
		reactor_unregister(queue->reactor_event);
		queue->reactor_event = NULL;
	}
}
