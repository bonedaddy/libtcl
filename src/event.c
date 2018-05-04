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

void equeue_init(equeue_t *equeue)
{
	bzero(equeue, sizeof(equeue_t));
}

void equeue_destroy(equeue_t *equeue)
{
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
	equeue_push_silent(equeue, ev);
	if (equeue->listener) {
		sched_spawn(equeue->sched, (work_t *)equeue->listener, 32, equeue, 1);
		fiber_yield(NULL);
	}
}

event_t *equeue_pop(equeue_t *equeue)
{
	event_t *ev;

	if (!equeue->slot) ev = NULL;
	else {
		ev = *equeue->buf;
		memmove(equeue->buf, equeue->buf + 1,
			--equeue->slot * sizeof(event_t));
	}
	return ev;
}

void equeue_listen(equeue_t *equeue, sched_t *sched, listener_t *listener)
{
	equeue->listener = listener;
	equeue->sched = sched;
}
