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

#include <osi/string.h>

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
#ifdef OS_PROVENCORE
	fib->context = create_context(ss, 0, 0, 0, (int (*)(void *))__fibfn, fib);
#else
	coro_stack_alloc(&fib->stack, ss);
	coro_create(&fib->context, (coro_func)__fibfn, fib, fib->stack.sptr,
		fib->stack.ssze);
#endif
	if (fn) osi_sched_ready(fib);
	return fib;
}

void osi_fiber_swap(osi_fib_t *from, osi_fib_t *to)
{
#ifdef OS_PROVENCORE
	int dummy;

	(void)from;
	if (resume(to->context, &dummy))
		to->status = OSI_FIB_EXITING;
#else
	if (from && to && from != to)
		coro_transfer(&from->context, &to->context);
#endif
}

void osi_fiber_delete(osi_fib_t *fib)
{
#ifdef OS_PROVENCORE
	fib->context = NULL;
#else
	(void)coro_destroy(&fib->context);
	coro_stack_free(&fib->stack);
#endif

}
