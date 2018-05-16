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

#include "osi/fiber.h"
#include "osi/string.h"

#include <errno.h>

static fiber_t __s_fiber = { };
static fiber_t *__fiber = &__s_fiber;

#ifdef OS_PROVENCORE
static int
#else
static void
#endif
__fn(fiber_t *fiber)
{
	fiber->result = fiber->fn(fiber->arg);
	fiber->status = FIBER_EXITING;
	fiber_yield(fiber->result);
#ifdef OS_PROVENCORE
	return 0;
#endif
}

#ifdef OS_PROVENCORE
static int
#else
static void
#endif
__fn_loop(fiber_t *fiber)
{
	while (fiber->status == FIBER_RUNNING)
		fiber_yield(fiber->result = fiber->fn(fiber->arg));
	fiber->status = FIBER_EXITING;
	fiber_yield(fiber->result);
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

void fiber_init(fiber_t *fiber, work_t *work, uint16_t ss, uint8_t flags)
{
#ifdef OS_PROVENCORE
	static int init = 0;
#endif

	bzero(fiber, sizeof(fiber_t));
	head_init(&fiber->hold);
	fiber->fn = work;
#ifdef OS_PROVENCORE
	if (!init) {
		init = 1;
		if (init_threads()) return;
	}
	fiber->ss = ss;
	fiber->context = create_context(ss, 0, 0, 0, __coro(flags), fiber);
#elif defined(USE_CORO)
	coro_stack_alloc(&fiber->stack, ss);
	coro_create(&fiber->context, __coro(flags), fiber, fiber->stack.sptr,
		fiber->stack.ssze);
#else
	fiber->ss = ss;
	fiber->context = coroutine(__coro(flags));
#endif
}

int fiber_reuse(fiber_t *fiber, work_t *fn, uint8_t flags)
{
	if (fiber->status != FIBER_EXITING) {
		errno = EINVAL;
		return -1;
	}
	if (fn) fiber->fn = fn;
#ifdef OS_PROVENCORE
	fiber->context = create_context(fiber->ss, 0, 0, 0, __coro(flags), fiber);
#elif defined(USE_CORO)
	(void)coro_destroy(&fiber->context);
	coro_create(&fiber->context, __coro(flags), fiber, fiber->stack.sptr,
		fiber->stack.ssze);
#else
	fiber->context = coroutine(__coro(flags));
#endif
	fiber->status = FIBER_READY;
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
	fiber->status = FIBER_RUNNING;
#ifdef USE_PICORO
	fiber->result = resume(fiber->context, fiber);
#else
	fiber->caller = __fiber;
	__fiber = fiber;
# ifdef OS_PROVENCORE
	int dummy;

	if (resume(fiber->context, &dummy))
		fiber->status = FIBER_EXITING;
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
	return fiber->status == FIBER_EXITING;
}

void fiber_join(fiber_t *fiber)
{
	while (!fiber_isdone(fiber))
		fiber_call(fiber, NULL);
}

void *fiber_yield(void *arg)
{
#ifdef USE_PICORO
	return yield(arg);
#else
	fiber_t *caller;
	fiber_t *fiber;

	if (!(fiber = __fiber))
		return NULL;
	if (!fiber->caller)
		return fiber_call(fiber, arg);

	fiber->result = arg;
	caller = fiber->caller;
	__fiber = caller;
	fiber->caller = NULL;

# ifdef OS_PROVENCORE
	if (caller) {
		int dummy;

		if (resume(caller->context, &dummy))
			caller->status = FIBER_EXITING;
	} else {
		yield();
	}
# else
	coro_transfer(&fiber->context, &caller->context);
# endif
	return fiber->arg;
#endif
}

fiber_t *fiber_current(void)
{
	return __fiber;
}
