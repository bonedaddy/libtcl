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

#define LOG_TAG "osi_fiber"

#include "osi/log.h"
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
} fiber_t;

static fid_t __fiber_current = 0;
static struct {
	fiber_t *buffer;
	uint16_t len;
	uint16_t cap;
} __fibers = {NULL, 0, 0};

static fiber_t *__getfiber(fid_t fid)
{
	if (fid >= __fibers.len) {
		errno = EINVAL;
		return NULL;
	}
	return __fibers.buffer + fid;
}

static fid_t __createfiber(void)
{
	fid_t i;
	fiber_t *fiber;
	uint16_t cap;

	for (i = 1; i < __fibers.len; ++i) {
		fiber = __getfiber(i);
		if (fiber->status == FIBER_DONE || fiber->status == FIBER_DESTROYED)
			goto got_one;
	}
	if ((i = __fibers.len++) + 1 >= __fibers.cap) {
		cap = (uint16_t)(__fibers.cap * 2);
		__fibers.buffer = realloc(__fibers.buffer, cap * sizeof(fiber_t));
		bzero(__fibers.buffer + __fibers.cap, (cap - __fibers.cap) *
			sizeof(fiber_t));
		__fibers.cap = cap;
	}
	got_one:
	__fibers.buffer[i].fid = i;
	return i;
}

#ifdef OS_PROVENCORE
# define CALLBACK_RETURN_TY int
#else
# define CALLBACK_RETURN_TY void
#endif

static CALLBACK_RETURN_TY __fn(fid_t *fid)
{
	fiber_t *fiber;

	fiber = __getfiber(*fid);
	fiber->result = fiber->fn(fiber->arg);
	fiber->status = FIBER_DONE;
	fiber_yield(fiber->result);
#ifdef OS_PROVENCORE
	return 0;
#endif
}

static CALLBACK_RETURN_TY __fn_loop(fid_t *fid)
{
	fiber_t *fiber;

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
	if ((flags & FIBER_FL_LOOP))
		return __fn_loop;
	return __fn;
}

void fiber_init(fid_t *fid, work_t *work, uint16_t ss, uint8_t flags)
{
	static int init = 0;
	fiber_t *fiber;

	if (!init) {
		init = 1;
		__fibers.buffer = calloc(32, sizeof(fiber_t));
		__fibers.len = 1;
		__fibers.cap = 32;
		__fibers.buffer[0].status = FIBER_RUNNING;
#ifdef OS_PROVENCORE
		if (init_threads()) return -1;
#endif
	}
	*fid = __createfiber();
	fiber = __getfiber(*fid);
#ifdef OS_PROVENCORE
	fiber->context = create_context(ss, 0, 0, 0, __coro(flags), fid);
#elif defined(USE_CORO)
	if (fiber->ss < ss) {
		coro_stack_free(&fiber->stack);
		coro_stack_alloc(&fiber->stack, ss);
	}
	coro_create(&fiber->context, __coro(flags), fid,
		fiber->stack.sptr, fiber->stack.ssze);
#else
	fiber->context = coroutine(__coro(flags));
#endif
	head_init(&fiber->hold);
	fiber->ss = ss;
	fiber->fn = work;
	fiber->status = FIBER_PENDING;
}

void fiber_destroy(fid_t fid)
{
	fiber_t *fiber;

	fiber = __getfiber(fid);
	assert(fiber->status == FIBER_DONE);
#ifdef USE_CORO
	(void)coro_destroy(&fiber->context);
	coro_stack_free(&fiber->stack);
#endif
	bzero(fiber, sizeof(fiber_t));
	fiber->fid = fid;
	fiber->status = FIBER_DESTROYED;
}

void fiber_cleanup(void)
{
	fid_t i;
	fiber_t *fiber;

	for (i = 1; i < __fibers.len; ++i) {
		fiber = __getfiber(i);
		if (fiber->status != FIBER_DESTROYED)
			fiber_destroy(i);
	}
	if (__fibers.len) {
		free(__fibers.buffer);
		bzero(&__fibers, sizeof(__fibers));
	}
}

void fiber_setcontext(fid_t fid, void *ctx)
{
	fiber_t *fiber;

	fiber = __getfiber(fid);
	fiber->arg = ctx;
}

void *fiber_call(fid_t fid, void *ctx)
{
	fiber_t *fiber;
	fiber_t *current;

	current = __getfiber(__fiber_current);
	if (current->caller == fid)
		return fiber_yield(ctx);
	fiber = __getfiber(fid);
	assert(__fiber_current != fid);
	assert(fiber->status == FIBER_PENDING);
	fiber->arg = ctx;
	fiber->caller = __fiber_current;
	__fiber_current = fiber->fid;
	fiber->status = FIBER_RUNNING;
	if (current->status == FIBER_RUNNING)
		current->status = FIBER_PENDING;
#ifdef USE_PICORO
	fiber->result = resume(fiber->context, &fid);
#elif defined(OS_PROVENCORE)
	int dummy;

	if (resume(fiber->context, &dummy))
		fiber->status = FIBER_DONE;
#else
	coro_transfer(&__getfiber(fiber->caller)->context, &fiber->context);
#endif
	return fiber->result;
}

bool fiber_isdone(fid_t fid)
{
	return __getfiber(fid)->status == FIBER_DONE;
}

static void __schedule(void)
{
	fid_t fid;
	fiber_t *fiber;

	if ((fid = (fid_t)(__fiber_current + 1)) >= __fibers.len)
		fid = 0;
	while ((fiber = __getfiber(fid))->status != FIBER_PENDING) {
		if (++fid >= __fibers.len)
			fid = 0;
	}
	fiber_call(fid, fiber->arg);
}

void fiber_join(fid_t fid)
{
	while (!fiber_isdone(fid))
		__schedule();
}

void *fiber_yield(void *arg)
{
	fiber_t *fiber;
	fiber_t *caller;

	fiber = __getfiber(__fiber_current);
	caller = __getfiber(fiber->caller);
	assert(fiber->status == FIBER_RUNNING
		|| fiber->status == FIBER_DONE
		|| fiber->status == FIBER_BLOCKING);
	assert(caller->status == FIBER_PENDING
		|| caller->status == FIBER_DONE);
	while (caller && caller->status == FIBER_DONE)
		caller = __getfiber(caller->caller);
	assert(caller);
	assert(caller->status == FIBER_PENDING);
	__fiber_current = caller->fid;
	fiber->caller = 0;
	if (fiber->status == FIBER_RUNNING)
		fiber->status = FIBER_PENDING;
	caller->status = FIBER_RUNNING;
#ifdef USE_PICORO
	return yield(arg);
#else
	fiber->result = arg;
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

void fiber_lock(void)
{
	fiber_t *fiber;

	fiber = __getfiber(__fiber_current);
	assert(fiber->status == FIBER_RUNNING);
	fiber->status = FIBER_BLOCKING;
	__schedule();
}

void fiber_unlock(fid_t fid)
{
	fiber_t *fiber;

	fiber = __getfiber(fid);
	assert(fiber->status == FIBER_BLOCKING);
	fiber->status = FIBER_PENDING;
}
