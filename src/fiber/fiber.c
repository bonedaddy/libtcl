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

#include "fiber.h"
#include "sched.h"

static void __fibfn(osi_fib_t *fib)
{
	fib->fn(fib->arg);
	__scheduler->running->status = OSI_FIB_EXITING;
	osi_sched_switch();
}

static void __fiber_ready(osi_fib_t *fiber)
{
	if (fiber->status != OSI_FIB_EXITING) {
		fiber->status = OSI_FIB_READY;
		osi_ring_prepend(&__scheduler->ready, &fiber->hold);
	}
}

osi_fib_t *osi_fib_create(osi_fibfn_t *fn, void *arg, uint16_t ss, uint8_t prio)
{
	osi_fib_t *fib;
	osi_ring_t *head;

	(void)ss; //TODO
	if ((head = osi_ring_pop(&__scheduler->dead)))
		fib = RING_ENTRY(head, osi_fib_t, hold);
	else {
		if (__scheduler->slot >= __scheduler->size) {
			__scheduler->size += 128;
			__scheduler->fibers = realloc(__scheduler->fibers,
				__scheduler->size * sizeof(osi_fib_t));
		}
		fib = __scheduler->fibers + __scheduler->slot++;
	}
	bzero(fib, sizeof(osi_fib_t));
	osi_ring_init(&fib->hold);
	fib->fn = fn;
	fib->arg = arg;
	fib->priotity = prio;
	coro_create(&fib->context, (coro_func)__fibfn, fib, ss);
	__fiber_ready(fib);
	return fib;
}
