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

#include <osi/fiber.h>

void fiber_ev_init(fiber_ev_t *ev, fiber_evk_t kind)
{
	list_init(&ev->pending);
	ev->kind = kind;
	ev->isset = false;
}

void fiber_ev_destroy(fiber_ev_t *ev)
{
	list_destroy(&ev->pending, NULL);
}

void fiber_ev_set(fiber_ev_t *ev)
{
	head_t *head;
	fiber_t *fiber;

	if (ev->isset)
		return;
	while (true) {
		if (!(head = list_shift(&ev->pending))) {
			ev->isset = true;
			break;
		}
		fiber = LIST_ENTRY(head, fiber_t, hold);
		fiber_call(fiber, NULL);
		if (ev->kind == FIBER_EV_AUTO) {
			/* There is no need to set the event in this case, because
			 * auto-reset event must be reset if somebody has been worked up.
			 */
			break;
		}
	}
}

void fiber_ev_reset(fiber_ev_t *ev)
{
	ev->isset = false;
}

void fiber_ev_wait(fiber_ev_t *ev)
{
	fiber_t *fiber;

	if (!ev->isset) {
		if ((fiber = fiber_current())) {
			list_push(&ev->pending, &fiber->hold);
			fiber_yield(NULL);
		}
	} else if (ev->kind == FIBER_EV_AUTO) {
		/* Auto-reset event should be reset if it is set in order to
		 * block subsequent ff_event_wait() calls.
		 */
		ev->isset = 0;
	}
}

bool fiber_ev_isset(fiber_ev_t *ev)
{
	return ev->isset;
}
