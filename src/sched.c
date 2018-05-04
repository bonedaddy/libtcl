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

void sched_init(sched_t *sched)
{
	bzero(sched, sizeof(sched_t));
	list_init(&sched->ready);
	list_init(&sched->dead);
}

/*
 * TODO: Insert by priority
 */
void sched_spawn(sched_t *sched, work_t *work, uint16_t ss, void *arg,
	int prio)
{
	fiber_t *fib;

	fiber_init(fib = malloc(sizeof(fiber_t)), work, ss, FIBER_NONE);
	fib->status = OSI_FIB_READY;
	fib->arg = arg;
	fib->priority = prio;
	list_unshift(&sched->ready, &fib->hold);
}

/*
 * TODO: Insert by priority
 */
void sched_loop(sched_t *sched, work_t *work, uint16_t ss, void *arg,
	int prio)
{
	fiber_t *fib;

	fiber_init(fib = malloc(sizeof(fiber_t)), work, ss, FIBER_LOOP);
	fib->status = OSI_FIB_READY;
	fib->arg = arg;
	fib->priority = prio;
	list_unshift(&sched->ready, &fib->hold);
}

void sched_start(sched_t *sched, bool loop)
{
	fiber_t *fib;
	node_t *head;

	if (sched->scheduled) {
		errno = EINVAL;
		return;
	}
	sched->scheduled = true;

	/* Schedule ready fibers */
	while (sched->scheduled) {
		if (!(head = list_shift(&sched->ready))) {
			if (loop) continue;
			break;
		}
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

	/* Release ready fibers */
	while ((head = list_shift(&sched->ready)) != NULL) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		fiber_destroy(fib);
		free(fib);
	}

	/* Release dead fibers */
	while ((head = list_shift(&sched->dead)) != NULL) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		fiber_destroy(fib);
		free(fib);
	}

	/* Release scheduler stacks */
	list_init(&sched->ready);
	list_init(&sched->dead);
	sched->scheduled = false;
}

void sched_stop(sched_t *sched)
{
	sched->scheduled = false;
}
