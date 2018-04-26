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

#include "internal.h"
#include "event.h"
#include "sched.h"

void osi_event_ctor(osi_event_t *event, osi_eventk_t kind)
{
	bzero(event, sizeof(osi_event_t));
	event->kind = kind;
}

void osi_event_dtor(osi_event_t *event)
{
	osi_fibers_dtor(&event->pending);
	bzero(event, sizeof(osi_event_t));
}

void osi_event_set(osi_event_t *event)
{
	osi_fiber_t *fiber;

	if (event->isset)
		return;
	while (1) {
		if (!osi_fibers_len(&event->pending)) {
			event->isset = 1;
			break;
		}
		fiber = osi_fibers_peek(&event->pending);
		osi_fibers_pop(&event->pending, 1);
		osi_sched_push(fiber);

		/* there is no need to set the event in this case,
		 * because autoreset event must be reset if somebody has been worked up.
		 */
		if (event->kind == OSI_EVENT_AUTO)
			break;
	}
}

void osi_event_reset(osi_event_t *event)
{
	event->isset = 0;
}

int osi_event_isset(osi_event_t *event)
{
	return event->isset;
}

void osi_event_wait(osi_event_t *event)
{
	osi_fiber_t *fiber;

	if (!event->isset) {
		fiber = osi_sched_current();
		if (fiber) {
			memcpy(osi_fibers_push(&event->pending, 1), fiber,
				sizeof(osi_fiber_t));
			osi_yield();
		}
	} else if (event->kind == OSI_EVENT_AUTO) {
		event->isset = 0;
	}
}
