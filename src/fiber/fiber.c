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

#ifdef OS_PROVENCORE
fiber_t *__fiber = NULL;
#else
static fiber_t __s_fiber = { };
fiber_t *__fiber = &__s_fiber;
#endif

static void __fibfn(fiber_t *fib)
{
	fib->result = fib->fn(fib->arg);
	fib->status = OSI_FIB_EXITING;
	fiber_yield(fib->result);
}

fiber_t *fiber_new(fiber_fn_t *fn, uint16_t ss)
{
#ifdef OS_PROVENCORE
	static int init = 0;
#endif
	fiber_t *fib;

	fib = malloc(sizeof(fiber_t));
	bzero(fib, sizeof(fiber_t));
	node_init(&fib->hold);
	fib->fn = fn;
#ifdef OS_PROVENCORE
	if (!init) {
		init = 1;
		if (init_threads()) return NULL;
	}
	fib->context = create_context(ss, 0, 0, 0, (int (*)(void *))__fibfn, fib);
#else
	coro_stack_alloc(&fib->stack, ss);
	coro_create(&fib->context, (coro_func)__fibfn, fib, fib->stack.sptr,
		fib->stack.ssze);
#endif
	return fib;
}

void fiber_del(fiber_t *fib)
{
#ifndef OS_PROVENCORE
	(void)coro_destroy(&fib->context);
	coro_stack_free(&fib->stack);
#endif
	free(fib);
}

void *fiber_call(fiber_t *fib, void *ctx)
{
	fib->caller = __fiber;
	fib->arg = ctx;
	fib->status = OSI_FIB_RUNNING;
	__fiber = fib;
#ifdef OS_PROVENCORE
	int dummy;

	if (resume(fib->context, &dummy))
		fib->status = OSI_FIB_EXITING;
#else
	if (fib->caller != fib) {
		coro_transfer(&fib->caller->context, &fib->context);
	}
#endif
	return fib->result;
}

bool fiber_isdone(fiber_t *fib)
{
	return fib->status == OSI_FIB_EXITING;
}

void *fiber_yield(void *arg)
{
	fiber_t *caller;
	fiber_t *fib;

	if (!(fib = __fiber))
		return NULL;
	fib->result = arg;
	caller = fib->caller;
	__fiber = caller;
	fib->caller = NULL;
	if (caller) {
#ifdef OS_PROVENCORE
		int dummy;

		if (resume(caller, &dummy))
			caller->status = OSI_FIB_EXITING;
#else
		coro_transfer(&fib->context, &caller->context);
#endif
	} else {
#ifdef OS_PROVENCORE
		yield();
#else
		errno = EINVAL;
		return NULL;
#endif
	}
	return fib->arg;
}
