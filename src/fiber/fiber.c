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
	fib->status = OSI_FIB_EXITING;
	osi_yield();
}

osi_fib_t *osi_fib_create(osi_fibfn_t *fn, void *arg, uint16_t ss, uint8_t prio)
{
	osi_fib_t *fib;

	fib = osi_sched_entry();
	bzero(fib, sizeof(osi_fib_t));
	osi_node_init(&fib->hold);
	fib->fn = fn;
	fib->arg = arg;
	fib->priotity = prio;
	coro_create(&fib->context, (coro_func)__fibfn, fib, ss);
	if (fn) osi_sched_ready(fib);
	return fib;
}

void osi_fiber_swap(osi_fib_t *from, osi_fib_t *to)
{
	if (from && to && from != to)
		coro_transfer(&from->context, &to->context);
}

void osi_fiber_delete(osi_fib_t *fib)
{
	coro_destroy(&fib->context);
}
