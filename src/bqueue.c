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

#include <osi/bqueue.h>
#include <osi/thread.h>
#include <osi/string.h>

int bqueue_init(bqueue_t *bqueue, unsigned capacity)
{
	bzero(bqueue, sizeof(bqueue_t));
	bqueue->capacity = capacity;
	if (sema_init(&bqueue->enqueue_sem, capacity))
		return -1;
	if (sema_init(&bqueue->dbqueue_sem, 0))
		return -1;
	list_init(&bqueue->list);
#ifdef OSI_THREADING
	pthread_mutex_init(&bqueue->lock, NULL);
#endif
	return 0;
}

void bqueue_destroy(bqueue_t *bqueue, head_dtor_t *dtor)
{
	bqueue_unlisten(bqueue);
	list_destroy(&bqueue->list, dtor);
	sema_destroy(&bqueue->enqueue_sem);
	sema_destroy(&bqueue->dbqueue_sem);
#ifdef OSI_THREADING
	pthread_mutex_destroy(&bqueue->lock);
#endif
}

bool bqueue_empty(bqueue_t *bqueue)
{
	bool empty;

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	empty = list_empty(&bqueue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#endif
	return empty;
}

unsigned bqueue_length(bqueue_t *bqueue)
{
	unsigned len;

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	len = (unsigned)bqueue->list.len;
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#endif
	return len;
}

unsigned bqueue_capacity(bqueue_t *bqueue)
{
	return bqueue->capacity;
}

void bqueue_push(bqueue_t *bqueue, head_t *ev)
{
	sema_wait(&bqueue->enqueue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	list_push(&bqueue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#else
	if (bqueue->listener) {
		sched_spawn(stdsched, (work_t *)bqueue->listener, 32, bqueue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&bqueue->dbqueue_sem);
}

head_t *bqueue_pop(bqueue_t *bqueue)
{
	head_t *ev;

	sema_wait(&bqueue->dbqueue_sem);

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	ev = list_shift(&bqueue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#endif
	sema_post(&bqueue->enqueue_sem);
	return ev;
}

bool bqueue_trypush(bqueue_t *bqueue, head_t *ev)
{
	if (!sema_trywait(&bqueue->enqueue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	list_push(&bqueue->list, ev);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#else
	if (bqueue->listener) {
		sched_spawn(stdsched, (work_t *)bqueue->listener, 32, bqueue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&bqueue->dbqueue_sem);
	return true;
}

head_t *bqueue_trypop(bqueue_t *bqueue)
{
	head_t *ev;

	if (!sema_trywait(&bqueue->dbqueue_sem))
		return false;

#ifdef OSI_THREADING
	pthread_mutex_lock(&bqueue->lock);
#endif
	ev = list_shift(&bqueue->list);
#ifdef OSI_THREADING
	pthread_mutex_unlock(&bqueue->lock);
#endif
	sema_post(&bqueue->enqueue_sem);
	return ev;
}

void bqueue_listen(bqueue_t *bqueue, thread_t *thread, listener_t *listener)
{
	bqueue->listener = listener;
#ifdef OSI_THREADING
	bqueue_unlisten(bqueue);
	bqueue->reactor_object = reactor_register(
		&thread->reactor, bqueue->dbqueue_sem.handle, bqueue,
		(reactor_ready_t *)listener, NULL);
#else
	bqueue->thread = thread;
#endif /* OSI_THREADING */
}

void bqueue_unlisten(bqueue_t *bqueue)
{
#ifdef OSI_THREADING
	if (bqueue->reactor_object) {
		reactor_unregister(bqueue->reactor_object);
		bqueue->reactor_object = NULL;
	}
#else
	bqueue->listener = NULL;
#endif /* OSI_THREADING */
}
