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

static struct {
	size_t slot;
	size_t size;
	osi_fib_t *buf;
} __fibers = { 0, 0, NULL, };
#ifdef OS_PROVENCORE
static osi_fib_t *__fiber = NULL;
#else
static osi_fib_t __s_fiber = { };
static osi_fib_t *__fiber = &__s_fiber;
#endif

static void __fibfn(osi_fib_t *fib)
{
	fib->result = fib->fn(fib->arg);
	fib->status = OSI_FIB_EXITING;
	osi_fib_yield(fib->result);
}

static osi_fib_t *__fib_allocate(void)
{
	if (__fibers.slot >= __fibers.size) {
		__fibers.size += 128;
		__fibers.buf = realloc(__fibers.buf,
			__fibers.size * sizeof(osi_fib_t));
	}
	return __fibers.buf + __fibers.slot++;
}

osi_fib_t *osi_fib_new(osi_fibfn_t *fn, uint16_t ss)
{
	osi_fib_t *fib;

	fib = __fib_allocate();
	bzero(fib, sizeof(osi_fib_t));
	osi_node_init(&fib->hold);
	fib->fn = fn;
#ifdef OS_PROVENCORE
	fib->context = create_context(ss, 0, 0, 0, (int (*)(void *))__fibfn, fib);
#else
	coro_stack_alloc(&fib->stack, ss);
	coro_create(&fib->context, (coro_func)__fibfn, fib, fib->stack.sptr,
		fib->stack.ssze);
#endif
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

void *osi_fib_call(osi_fib_t *fib, void *ctx)
{
	fib->caller = __fiber;
	fib->arg = ctx;
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

bool osi_fib_done(osi_fib_t *fib)
{
	return fib->status == OSI_FIB_EXITING;
}

void *osi_fib_yield(void *arg)
{
	osi_fib_t *caller;
	osi_fib_t *fib;

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
