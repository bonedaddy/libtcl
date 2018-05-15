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
#include <osi/string.h>

#include <errno.h>

#ifdef OS_PROVENCORE
static fiber_t *__fiber = NULL;
#elif defined(USE_CORO)
static fiber_t __s_fiber = { };
static fiber_t *__fiber = &__s_fiber;
#endif

#ifdef OS_PROVENCORE
static int
#else
static void
#endif
__fn(fiber_t *fib)
{
	fib->result = fib->fn(fib->arg);
	fib->status = OSI_FIB_EXITING;
	fiber_yield(fib->result);
#ifdef OS_PROVENCORE
	return 0;
#endif
}

#ifdef OS_PROVENCORE
static int
#else
static void
#endif
__fn_loop(fiber_t *fib)
{
	while (fib->status == OSI_FIB_RUNNING)
		fiber_yield(fib->result = fib->fn(fib->arg));
	fib->status = OSI_FIB_EXITING;
	fiber_yield(fib->result);
#ifdef OS_PROVENCORE
	return 0;
#endif
}

static void *__coro(uint8_t flags)
{
	if ((flags & FIBER_LOOP))
		return __fn_loop;
	return __fn;
}

void fiber_init(fiber_t *fib, work_t *fn, uint16_t ss, uint8_t flags)
{
#ifdef OS_PROVENCORE
	static int init = 0;
#endif

	bzero(fib, sizeof(fiber_t));
	node_init(&fib->hold);
	fib->fn = fn;
#ifdef OS_PROVENCORE
	if (!init) {
		init = 1;
		if (init_threads()) return;
	}
	fib->ss = ss;
	fib->context = create_context(ss, 0, 0, 0, __coro(flags), fib);
#elif defined(USE_CORO)
	coro_stack_alloc(&fib->stack, ss);
	coro_create(&fib->context, __coro(flags), fib, fib->stack.sptr,
		fib->stack.ssze);
#else
	fib->ss = ss;
	fib->context = coroutine(__coro(flags));
#endif
}

int fiber_reuse(fiber_t *fib, work_t *fn, uint8_t flags)
{
	if (fib->status != OSI_FIB_EXITING) {
		errno = EINVAL;
		return -1;
	}
	if (fn) fib->fn = fn;
#ifdef OS_PROVENCORE
	fib->context = create_context(fib->ss, 0, 0, 0, __coro(flags), fib);
#elif defined(USE_CORO)
	(void)coro_destroy(&fib->context);
	coro_create(&fib->context, __coro(flags), fib, fib->stack.sptr,
		fib->stack.ssze);
#else
	fib->context = coroutine(__coro(flags));
#endif
	fib->status = OSI_FIB_READY;
	return 0;
}

void fiber_destroy(fiber_t *fiber)
{
#ifdef USE_CORO
	(void)coro_destroy(&fiber->context);
	coro_stack_free(&fiber->stack);
#else
	(void)fiber;
#endif
}

void *fiber_call(fiber_t *fiber, void *ctx)
{
	fiber->arg = ctx;
	fiber->status = OSI_FIB_RUNNING;
#ifdef USE_PICORO
	fiber->result = resume(fiber->context, fiber);
#else
	fiber->caller = __fiber;
	__fiber = fiber;
# ifdef OS_PROVENCORE
	int dummy;

	if (resume(fiber->context, &dummy))
		fiber->status = OSI_FIB_EXITING;
# else
	if (fiber->caller != fiber) {
		coro_transfer(&fiber->caller->context, &fiber->context);
	}
# endif
#endif
	return fiber->result;
}

bool fiber_isdone(fiber_t *fiber)
{
	return fiber->status == OSI_FIB_EXITING;
}

void *fiber_yield(void *arg)
{
#ifdef USE_PICORO
	return yield(arg);
#else
	fiber_t *caller;
	fiber_t *fib;

	if (!(fib = __fiber) || !fib->caller)
		return NULL;

	fib->result = arg;
	caller = fib->caller;
	__fiber = caller;
	fib->caller = NULL;

# ifdef OS_PROVENCORE
	if (caller) {
		int dummy;

		if (resume(caller->context, &dummy))
			caller->status = OSI_FIB_EXITING;
	} else {
		yield();
	}
# else
	coro_transfer(&fib->context, &caller->context);
# endif
	return fib->arg;
#endif
}
