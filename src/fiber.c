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

typedef struct {

	/*! The fiber status */
	fiber_st_t status;

	/*! The fiber core function */
	work_t *fn;

	/*! The fiber core function argument */
	void *arg;

	/*! The fiber result */
	void *result;

	/*! The fiber caller */
	fid_t caller;

	/*! Fiber list hold */
	head_t hold;

	/*! The priority used by scheduler */
	int priority;

	fid_t fid;

	uint16_t ss;

#if defined(USE_CORO)

	/** Coroutine context */
	coro_context context;

	/** Coroutine stack */
	struct coro_stack stack;
#elif defined(USE_PICORO)

	/** Picoro coroutine context */
	coro context;
#elif defined(OS_PROVENCORE)

	/** ProveNCore coroutine context */
	struct context *context;
#endif
} fiber_impl_t;

static fid_t __fiber_current = 0;
static struct {
	fiber_impl_t *buffer;
	uint16_t len;
	uint16_t cap;
} __fibers = { NULL, 0, 0 };

static fiber_impl_t *__getfiber(fid_t fid)
{
	if (!__fibers.len) {
		__fibers.buffer = calloc(32, sizeof(fiber_impl_t));
		__fibers.len = 1;
		__fibers.cap = 32;
	}
	if (fid >= __fibers.len) {
		errno = EINVAL;
		return NULL;
	}
	return __fibers.buffer + fid;
}

static fid_t __createfiber(void)
{
	fid_t i;
	fiber_impl_t *fiber;
	uint16_t cap;

	for (i = 1; i < __fibers.len; ++i) {
		fiber = __getfiber(i);
		if (fiber->status == FIBER_DONE || fiber->status == FIBER_DESTROYED)
			goto got_one;
	}
	if (!__fibers.len) {
		__fibers.buffer = calloc(32, sizeof(fiber_impl_t));
		__fibers.len = 1;
		__fibers.cap = 32;
	}
	if ((i = __fibers.len++) + 1 >= __fibers.cap) {
		cap = (uint16_t)(__fibers.cap * 2);
		__fibers.buffer = realloc(__fibers.buffer, cap * sizeof(fiber_impl_t));
		bzero(__fibers.buffer + __fibers.cap, (cap - __fibers.cap) *
			sizeof(fiber_impl_t));
		__fibers.cap = cap;
	}
got_one:
	__fibers.buffer[i].fid = i;
	return i;
}

#ifdef OS_PROVENCORE
static int
#else
static void
#endif
__fn(fid_t *fid)
{
	fiber_impl_t *fiber;

	fiber = __getfiber(*fid);
	fiber->result = fiber->fn(fiber->arg);
	fiber->status = FIBER_DONE;
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
__fn_loop(fid_t *fid)
{
	fiber_impl_t *fiber;

	fiber = __getfiber(*fid);
	while (fiber->status == FIBER_RUNNING)
		fiber_yield(fiber->result = fiber->fn(fiber->arg));
	fiber->status = FIBER_DONE;
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

void fiber_init(fid_t *fid, work_t *work, uint16_t ss, uint8_t flags)
{
#ifdef OS_PROVENCORE
	static int init = 0;
#endif
	fiber_impl_t *impl;

	*fid = __createfiber();
	impl = __getfiber(*fid);
#ifdef OS_PROVENCORE
	if (!init) {
		init = 1;
		if (init_threads()) return -1;
	}
	impl->context = create_context(ss, 0, 0, 0, __coro(flags), fid);
#elif defined(USE_CORO)
	if (impl->ss < ss) {
		coro_stack_free(&impl->stack);
		coro_stack_alloc(&impl->stack, ss);
	}
	coro_create(&impl->context, __coro(flags), fid,
		impl->stack.sptr, impl->stack.ssze);
#else
	impl->context = coroutine(__coro(flags));
#endif
	head_init(&impl->hold);
	impl->ss = ss;
	impl->fn = work;
	impl->status = FIBER_PENDING;
}

void fiber_destroy(fid_t fid)
{
	fiber_impl_t *impl;

	impl = __getfiber(fid);
	assert(impl->status == FIBER_DONE);
#ifdef USE_CORO
	(void)coro_destroy(&impl->context);
	coro_stack_free(&impl->stack);
#endif
	bzero(impl, sizeof(fiber_impl_t));
	impl->fid = fid;
	impl->status = FIBER_DESTROYED;
}

void fiber_setcontext(fid_t fid, void *ctx)
{
	fiber_impl_t *impl;

	impl = __getfiber(fid);
	impl->arg = ctx;
}

void *fiber_call(fid_t fid, void *ctx)
{
	fiber_impl_t *impl;

	impl = __getfiber(fid);
	assert(__fiber_current != fid);
	assert(impl->status == FIBER_PENDING);
	impl->arg = ctx;
	impl->status = FIBER_RUNNING;
#ifdef USE_PICORO
	impl->result = resume(impl->context, &fid);
#else
	impl->caller = __fiber_current;
	__fiber_current = impl->fid;
# ifdef OS_PROVENCORE
	int dummy;

	if (resume(impl->context, &dummy))
		impl->status = FIBER_DONE;
# else
	coro_transfer(&__getfiber(impl->caller)->context, &impl->context);
# endif
#endif
	return impl->result;
}

bool fiber_isdone(fid_t fid)
{
	return __getfiber(fid)->status == FIBER_DONE;
}

void fiber_join(fid_t fid)
{
	while (!fiber_isdone(fid))
		fiber_call(fid, NULL);
}

void *fiber_yield(void *arg)
{
	fiber_impl_t *fiber;

	fiber = __getfiber(__fiber_current);
	assert(fiber->status == FIBER_RUNNING
		|| fiber->status == FIBER_DONE
		|| fiber->status == FIBER_BLOCKING);
	if (fiber->status == FIBER_RUNNING)
		fiber->status = FIBER_PENDING;
#ifdef USE_PICORO
	return yield(arg);
#else
	fiber_impl_t *caller;

	fiber->result = arg;
	caller = __getfiber(fiber->caller);
	__fiber_current = caller->fid;
	fiber->caller = 0;
# ifdef OS_PROVENCORE
	if (caller) {
		int dummy;

		if (resume(caller->context, &dummy))
			caller->status = FIBER_DONE;
	} else {
		yield();
	}
# else
	coro_transfer(&fiber->context, &caller->context);
# endif
	return fiber->arg;
#endif
}

fid_t fiber_getfid(void)
{
	return __fiber_current;
}

void fiber_schedule(void)
{
	fid_t i;
	fiber_impl_t *fiber;

	for (i = 1; i < __fibers.len; ++i) {
		fiber = __getfiber(i);
		if (fiber->status == FIBER_PENDING)
			goto got_one;
	}
	if (__fiber_current)
		fiber_yield(NULL);
	return;
got_one:
	fiber = __getfiber(i);
	fiber_call(fiber->fid, fiber->arg);
}

void fiber_lock(void)
{
	fid_t fid;
	fiber_impl_t *fiber;

	fid = fiber_getfid();
	fiber = __getfiber(fid);
	if (!fid) {
		fiber_schedule();
	} else {
		assert(fiber->status == FIBER_RUNNING);
		fiber->status = FIBER_BLOCKING;
		fiber_yield(NULL);
	}
}

void fiber_unlock(fid_t fid)
{
	fiber_impl_t *fiber;

	fiber = __getfiber(fid);
	if (!fid) {
		fiber_schedule();
	} else {
		assert(fiber->status == FIBER_BLOCKING);
		fiber->status = FIBER_PENDING;
		fiber_call(fid, fiber->arg);
	}
}
