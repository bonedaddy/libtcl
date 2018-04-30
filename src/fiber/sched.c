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

#include "sched.h"

typedef struct osi_sched osi_sched_t;

struct osi_sched {
	osi_fib_t *fibers;
	size_t size;
	size_t slot;

	osi_list_t ready;
	osi_list_t dead;

	osi_fib_t *running;
	osi_fib_t *root;
};

static osi_sched_t __s_scheduler = {
	.fibers = NULL, .size = 0, .slot = 0,
	.ready = LIST_INITIALIZER(__s_scheduler.ready),
	.dead = LIST_INITIALIZER(__s_scheduler.dead),
	.running = NULL,
	.root = NULL,
};

static osi_sched_t *__scheduler = &__s_scheduler;
static int __scheduled = 0;

osi_fib_t *osi_sched_entry(void)
{
	osi_fib_t *fib;
	osi_node_t *head;

	if ((head = osi_list_pop(&__scheduler->dead)))
		fib = LIST_ENTRY(head, osi_fib_t, hold);
	else {
		if (__scheduler->slot >= __scheduler->size) {
			__scheduler->size += 128;
			__scheduler->fibers = realloc(__scheduler->fibers,
				__scheduler->size * sizeof(osi_fib_t));
		}
		fib = __scheduler->fibers + __scheduler->slot++;
	}
	return (fib);
}

/*
 * TODO: Insert by priority
 */
void osi_sched_ready(osi_fib_t *fib)
{
	fib->status = OSI_FIB_READY;
	osi_list_unshift(&__scheduler->ready, &fib->hold);
}

void osi_schedule(void)
{
	osi_fib_t *fib;
	osi_node_t *head;

	if (__scheduled)
		return;
	__scheduled = 1;
	if (!__scheduler->root)
		__scheduler->root = osi_fib_create(NULL, NULL, 16, 0);

	/* Schedule ready fibers */
	while (1) {
		if (!(head = osi_list_shift(&__scheduler->ready)))
			break;
		fib = LIST_ENTRY(head, osi_fib_t, hold);
		fib->status = OSI_FIB_RUNNING;

		__scheduler->running = fib;
		osi_fiber_swap(__scheduler->root, fib);
		if (fib->status == OSI_FIB_EXITING)
			osi_list_unshift(&__scheduler->dead, &fib->hold);
		else
			osi_sched_ready(fib);
		__scheduler->running = NULL;
	}

	/* Release dead fibers */
	osi_fiber_delete(__scheduler->root);
	while ((head = osi_list_shift(&__scheduler->dead)) != NULL) {
		fib = LIST_ENTRY(head, osi_fib_t, hold);
		osi_fiber_delete(fib);
	}

	/* Release scheduler memory */
	free(__scheduler->fibers);
	__scheduler->root = NULL;
	__scheduler->fibers = NULL;
	__scheduler->size = 0;
	__scheduler->slot = 0;
	osi_list_init(&__scheduler->ready);
	osi_list_init(&__scheduler->dead);
	__scheduled = 0;
}

void osi_yield(void)
{
	osi_fiber_swap(__scheduler->running, __scheduler->root);
}
