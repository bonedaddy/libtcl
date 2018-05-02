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

#include <osi/sched.h>
#include <osi/string.h>

#include <errno.h>

#include "fiber.h"

void sched_init(sched_t *sched)
{
	bzero(sched, sizeof(sched_t));
	list_init(&sched->ready);
	list_init(&sched->dead);
}

/*
 * TODO: Insert by priority
 */
void sched_spawn(sched_t *sched, fiber_t *fib, void *arg, int prio)
{
	(void)prio;
	fib->status = OSI_FIB_READY;
	fib->arg = arg;
	list_unshift(&sched->ready, &fib->hold);
}

void sched_start(sched_t *sched)
{
	fiber_t *fib;
	node_t *head;

	if (sched->scheduled) {
		errno = EINVAL;
		return;
	}
	sched->scheduled = 1;

	/* Schedule ready fibers */
	while (1) {
		if (!(head = list_shift(&sched->ready)))
			break;
		fib = LIST_ENTRY(head, fiber_t, hold);
		fib->status = OSI_FIB_RUNNING;

		fiber_call(fib, fib->arg);
		if (fib->status == OSI_FIB_EXITING)
			list_unshift(&sched->dead, &fib->hold);
		else {
			fib->status = OSI_FIB_READY;
			list_unshift(&sched->ready, &fib->hold);
		}
	}

	/* Release dead fibers */
	while ((head = list_shift(&sched->dead)) != NULL) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		fiber_del(fib);
	}

	/* Release scheduler stacks */
	list_init(&sched->ready);
	list_init(&sched->dead);
	sched->scheduled = 0;
}
