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

#include <osi/equeue.h>
#include <osi/thread.h>
#include <osi/string.h>

int equeue_init(equeue_t *equeue, unsigned capacity)
{
	bzero(equeue, sizeof(equeue_t));
	equeue->capacity = capacity;
	if (sema_init(&equeue->enqueue_sem, capacity))
		return -1;
	if (sema_init(&equeue->dequeue_sem, 0))
		return -1;
	list_init(&equeue->list);
#ifdef OSI_THREADING
	pthread_mutex_init(&equeue->lock, NULL);
#endif
	return 0;
}

void equeue_destroy(equeue_t *equeue, node_dtor_t *dtor)
{
	equeue_unlisten(equeue);
	list_destroy(&equeue->list, dtor);
	sema_destroy(&equeue->enqueue_sem);
	sema_destroy(&equeue->dequeue_sem);
#ifdef OSI_THREADING
	pthread_mutex_destroy(&equeue->lock);
#endif
}

bool equeue_empty(equeue_t *equeue)
{
	bool empty;

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	empty = list_empty(&equeue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#endif
	return empty;
}

size_t equeue_length(equeue_t *equeue)
{
	size_t len;

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	len = equeue->list.len;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#endif
	return len;
}

void equeue_push(equeue_t *equeue, node_t *ev)
{
	sema_wait(&equeue->enqueue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	list_push(&equeue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#else
	if (equeue->listener) {
		sched_spawn(stdsched, (work_t *)equeue->listener, 32, equeue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&equeue->dequeue_sem);
}

node_t *equeue_pop(equeue_t *equeue)
{
	node_t *ev;

	sema_wait(&equeue->dequeue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	ev = list_shift(&equeue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#endif
	sema_post(&equeue->enqueue_sem);
	return ev;
}

bool equeue_trypush(equeue_t *equeue, node_t *ev)
{
	if (!sema_trywait(&equeue->enqueue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	list_push(&equeue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#else
	if (equeue->listener) {
		sched_spawn(stdsched, (work_t *)equeue->listener, 32, equeue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&equeue->dequeue_sem);
	return true;
}

node_t *equeue_trypop(equeue_t *equeue)
{
	node_t *ev;

	if (!sema_trywait(&equeue->dequeue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&equeue->lock);
#endif
	ev = list_shift(&equeue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&equeue->lock);
#endif
	sema_post(&equeue->enqueue_sem);
	return ev;
}

void equeue_listen(equeue_t *equeue, thread_t *thread, listener_t *listener)
{
	equeue->listener = listener;
#ifdef OSI_THREADING
	equeue_unlisten(equeue);
	equeue->reactor_object = reactor_register(
		&thread->reactor, equeue->dequeue_sem.handle, equeue,
		(reactor_ready_t *)listener, NULL);
#endif /* OSI_THREADING */
}

void equeue_unlisten(equeue_t *equeue)
{
#ifdef OSI_THREADING
	if (equeue->reactor_object) {
		reactor_unregister(equeue->reactor_object);
		equeue->reactor_object = NULL;
	}
#else
	equeue->listener = NULL;
#endif /* OSI_THREADING */
}
