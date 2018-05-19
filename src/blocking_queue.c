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
#include "osi/thread.h"
#include "osi/string.h"

int blocking_queue_init(blocking_queue_t *queue, unsigned capacity)
{
	bzero(queue, sizeof(blocking_queue_t));
	queue->capacity = capacity;
	if (sema_init(&queue->producer, capacity))
		return -1;
	if (sema_init(&queue->consumer, 0))
		return -1;
	/* TODO: use queue_t */
	list_init(&queue->list);
#ifdef OSI_THREADING
	pthread_mutex_init(&queue->lock, NULL);
#endif
	return 0;
}

void blocking_queue_destroy(blocking_queue_t *queue, head_dtor_t *dtor)
{
	blocking_queue_unlisten(queue);
	list_destroy(&queue->list, dtor);
	sema_destroy(&queue->producer);
	sema_destroy(&queue->consumer);
#ifdef OSI_THREADING
	pthread_mutex_destroy(&queue->lock);
#endif
}

bool blocking_queue_empty(blocking_queue_t *queue)
{
	bool empty;

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	empty = list_empty(&queue->list);
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
	len = (unsigned)queue->list.len;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	return len;
}

unsigned blocking_queue_capacity(blocking_queue_t *queue)
{
	return queue->capacity;
}

void blocking_queue_push(blocking_queue_t *queue, head_t *ev)
{
	sema_wait(&queue->producer);

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	list_push(&queue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->consumer);
}

head_t *blocking_queue_pop(blocking_queue_t *queue)
{
	head_t *ev;

	sema_wait(&queue->consumer);

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	ev = list_shift(&queue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->producer);
	return ev;
}

bool blocking_queue_trypush(blocking_queue_t *queue, head_t *node)
{
	if (!sema_trywait(&queue->producer))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	list_push(&queue->list, node);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->consumer);
	return true;
}

head_t *blocking_queue_trypop(blocking_queue_t *queue)
{
	head_t *ev;

	if (!sema_trywait(&queue->consumer))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&queue->lock);
#endif
	ev = list_shift(&queue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&queue->lock);
#endif
	sema_post(&queue->producer);
	return ev;
}

void blocking_queue_listen(blocking_queue_t *queue, thread_t *thread, listener_t *listener)
{
	queue->listener = listener;
#ifdef OSI_THREADING
	blocking_queue_unlisten(queue);
	queue->reactor_object = reactor_register(
		&thread->reactor, queue->consumer.handle, queue,
		(reactor_ready_t *)listener, NULL);
#else
	queue->thread = thread;
#endif /* OSI_THREADING */
}

void blocking_queue_unlisten(blocking_queue_t *queue)
{
#ifdef OSI_THREADING
	if (queue->reactor_object) {
		reactor_unregister(queue->reactor_object);
		queue->reactor_object = NULL;
	}
#else
	queue->listener = NULL;
#endif /* OSI_THREADING */
}
