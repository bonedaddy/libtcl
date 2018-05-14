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

#include <osi/event.h>
#include <osi/thread.h>
#include <osi/string.h>

int equeue_init(equeue_t *equeue)
{
	bzero(equeue, sizeof(equeue_t));
	if (sema_init(&equeue->enqueue_sem, 0))
		return -1;
	if (sema_init(&equeue->dequeue_sem, 0))
		return -1;
#ifdef OSI_THREAD_MOD
	pthread_mutex_init(&equeue->lock, NULL);
#endif
	return 0;
}

void equeue_destroy(equeue_t *equeue)
{
	if (equeue->reactor_object) {
		reactor_unregister(equeue->reactor_object);
		equeue->reactor_object = NULL;
	}
	sema_destroy(&equeue->enqueue_sem);
	sema_destroy(&equeue->dequeue_sem);
#ifdef OSI_THREAD_MOD
	pthread_mutex_destroy(&equeue->lock);
#endif
	if (equeue->buf) {
		free(equeue->buf);
		bzero(equeue, sizeof(equeue_t));
	}
}

void equeue_push_silent(equeue_t *equeue, event_t *ev)
{
	if (equeue->size < equeue->slot + 1) {
		equeue->size = equeue->size ? equeue->size * 2 : 32;
		equeue->buf = realloc(equeue->buf, equeue->size);
	}
	*(equeue->buf + equeue->slot++) = ev;
}

void equeue_push(equeue_t *equeue, event_t *ev)
{
	sema_wait(&equeue->enqueue_sem);
#ifdef OSI_THREAD_MOD
	pthread_mutex_lock(&equeue->lock);
#endif
	equeue_push_silent(equeue, ev);
#ifdef OSI_THREAD_MOD
	pthread_mutex_unlock(&equeue->lock);
#else
	if (equeue->listener) {
		sched_spawn(stdsched, (work_t *)equeue->listener, 32, equeue, 1);
		fiber_yield(NULL);
	}
#endif
	sema_post(&equeue->dequeue_sem);
}

event_t *equeue_pop(equeue_t *equeue)
{
	event_t *ev;

	sema_wait(&equeue->dequeue_sem);
#ifdef OSI_THREAD_MOD
	pthread_mutex_lock(&equeue->lock);
#endif
	if (!equeue->slot) ev = NULL;
	else {
		ev = *equeue->buf;
		memmove(equeue->buf, equeue->buf + 1,
			--equeue->slot * sizeof(event_t));
	}
#ifdef OSI_THREAD_MOD
	pthread_mutex_unlock(&equeue->lock);
#endif
	sema_post(&equeue->enqueue_sem);
	return ev;
}

void equeue_listen(equeue_t *equeue, thread_t *thread, listener_t *listener)
{
	equeue->listener = listener;
#ifdef OSI_THREAD_MOD
	if (equeue->reactor_object) {
		reactor_unregister(equeue->reactor_object);
		equeue->reactor_object = NULL;
	}
	equeue->reactor_object = reactor_register(
		&thread->reactor, equeue->dequeue_sem.handle, equeue,
		(work_t *)listener, NULL);
#endif /* OSI_THREAD_MOD */
}
