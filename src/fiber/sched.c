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

static osi_sched_t __s_scheduler = {
	.fibers = NULL, .size = 0, .slot = 0,
	.ready = RING_INITIALIZER(__s_scheduler.ready),
	.dead = RING_INITIALIZER(__s_scheduler.dead),
	.running = NULL,
	.root = NULL,
};

osi_sched_t *__scheduler = &__s_scheduler;
int __scheduled = 0;

static void __fibswap(osi_fib_t *from, osi_fib_t *to)
{
	if (from->status == OSI_FIB_EXITING)
		osi_ring_prepend(&__scheduler->dead, &from->hold);
	coro_transfer(&from->context, &to->context);
}

void osi_sched_switch(void)
{
	osi_fib_t *fib, *cur;
	osi_ring_t *head;

	cur = __scheduler->running;

	if (!(head = osi_ring_shift(&__scheduler->ready)))
		__fibswap(cur, __scheduler->root);
	else {
		fib = RING_ENTRY(head, osi_fib_t, hold);
		__scheduler->running = fib;
		__fibswap(cur, fib);
		__scheduler->running = NULL;
	}
}

void osi_schedule(void)
{
	osi_fib_t *fib;
	osi_ring_t *head;

	if (__scheduled)
		return;
	__scheduled = 1;
	if (!__scheduler->root)
		__scheduler->root = osi_fib_create(NULL, NULL, 16, 0);
	while (1) {
		if (!(head = osi_ring_shift(&__scheduler->ready)))
			break;
		fib = RING_ENTRY(head, osi_fib_t, hold);
		fib->status = OSI_FIB_RUNNING;

		__scheduler->running = fib;
		__fibswap(__scheduler->root, fib);
		__scheduler->running = NULL;
	}
	__scheduled = 0;
}

void osi_yield(void)
{
	if (!__scheduler->running)
		return;
	if (__scheduler->running->status != OSI_FIB_EXITING)
	{
		__scheduler->running->status = OSI_FIB_READY;
		osi_ring_prepend(&__scheduler->ready, &__scheduler->running->hold);
	}
	osi_sched_switch();
}
