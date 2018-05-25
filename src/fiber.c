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
#include "osi/vector.h"

#include <errno.h>

#if defined(USE_CORO)
# include <coro.h>
#elif defined(USE_PICORO)
# include <picoro.h>
#elif defined(OS_PROVENCORE)
# include <threads/threads.h>
#else
# error "Unable to implement software coroutines"
#endif

#define DEFAULT_FIBER_STACK_SIZE (4096)
#ifdef OS_PROVENCORE
# define CALLBACK_RETURN_TY int
#else
# define CALLBACK_RETURN_TY void
#endif

typedef struct {

	/*! The fiber status */
	fiber_st_t status;

	/*! The fiber core function */
	work_t *fn;

	/*! The fiber core function argument */
	void *context;

	/*! The fiber result */
	void *result;

	/*! The fiber caller */
	fid_t caller;

	/*! The priority used by scheduler */
	int priority;

	/*! The index in the __fibers.queue buffer. */
	uint16_t priority_idx;

	/*! Fiber unique id. */
	fid_t fid;

	/*! Fiber stack size. */
	uint16_t ss;

	/*! Used to pass fiber id to coroutine. */
	fid_t *coroutine_ctx;

#if defined(USE_CORO)

	/** Coroutine context */
	coro_context coroutine;

	/** Coroutine stack */
	struct coro_stack stack;
#elif defined(USE_PICORO)

	/** Picoro coroutine */
	coro coroutine;
#elif defined(OS_PROVENCORE)

	/** ProveNCore coroutine */
	struct coroutine *coroutine;
#endif
} fiber_t;

static fid_t __fiber_current = 0;
static uint16_t __fiber_idx = 0;
static struct {
	fiber_t *buffer;
	fid_t *queue;
	uint16_t len;
	uint16_t cap;
} __fibers = {NULL, 0, 0};

static int __lazyinit(void)
{
	static int init = 0;

	if (!init) {
		init = 1;
		__fibers.buffer = calloc(32, sizeof(fiber_t));
		__fibers.queue = calloc(32, sizeof(fid_t));
		__fibers.len = 1;
		__fibers.cap = 32;
		__fibers.buffer[0].status = FIBER_RUNNING;
		__fibers.queue[0] = 0;
#ifdef OS_PROVENCORE
		if (init_threads()) return -1;
#endif
	}
	return 0;
}

static __always_inline fiber_t *__getfiber(fid_t fid)
{
	__lazyinit();
	assert(fid < __fibers.len);
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
		__fibers.queue = realloc(__fibers.queue, cap * sizeof(fid_t));
		bzero(__fibers.queue + __fibers.cap, (cap - __fibers.cap) *
			sizeof(fid_t));
		__fibers.cap = cap;
	}
	got_one:
	__fibers.buffer[i].fid = i;
	return i;
}

static CALLBACK_RETURN_TY __fn(fid_t *fid)
{
	fiber_t *fiber;

	fiber = __getfiber(*fid);
	fiber->result = fiber->fn(fiber->context);
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
		fiber_yield(fiber->result = fiber->fn(fiber->context));
	fiber->status = FIBER_DONE;
	fiber_yield(fiber->result);
#ifdef OS_PROVENCORE
	return 0;
#endif
}

static __always_inline void __setpriority(fid_t fid)
{
	fid_t i;
	fiber_t *a, *b;

	a = __getfiber(fid);
	for (i = 1; i < __fibers.len; ++i) {
		if (!__fibers.queue[i])
			break;
		b = __getfiber(__fibers.queue[i]);
		if (b->priority < a->priority)
			break;
	}
	if (i < __fibers.len - 1)
		memmove(__fibers.queue + i + 1, __fibers.queue + i,
			(__fibers.len - 1 - i) * sizeof(fid_t));
	__fibers.queue[i] = fid;
	a->priority_idx = i;
}

static __always_inline void __updatepriority(fid_t fid)
{
	fid_t i;
	fiber_t *b;

	for (i = 1; i < __fibers.len; ++i) {
		b = __getfiber(__fibers.queue[i]);
		if (b->fid == fid)
			break;
	}
	memmove(__fibers.queue + i, __fibers.queue + i + 1,
		(__fibers.len - 1 - i) * sizeof(fid_t));
	__fibers.queue[i] = 0;
	__setpriority(fid);
}

void fiber_init(fid_t *fid, work_t *work, fiber_attr_t attr)
{
	fiber_t *fiber;
	void *fn;

	assert(!__lazyinit()); /* TODO: log an error and return if fail */
	*fid = __createfiber();
	fiber = __getfiber(*fid);
	if (!attr.stack_size)
		attr.stack_size = (uint16_t)DEFAULT_FIBER_STACK_SIZE;
	fn = (attr.flags & FIBER_FL_LOOP) ? __fn_loop : __fn;
	if (!fiber->coroutine_ctx)
		fiber->coroutine_ctx = malloc(sizeof(fid_t));
	*fiber->coroutine_ctx = *fid;
#ifdef OS_PROVENCORE
	fiber->coroutine = create_context(ss, 0, 0, 0, fn, fiber->coroutine_ctx);
#elif defined(USE_CORO)
	if (fiber->ss < attr.stack_size) {
		coro_stack_free(&fiber->stack);
		coro_stack_alloc(&fiber->stack, attr.stack_size);
	}
	coro_create(&fiber->coroutine, fn, fiber->coroutine_ctx, fiber->stack.sptr,
		fiber->stack.ssze);
#else
	fiber->coroutine = coroutine(fn);
#endif
	fiber->fn = work;
	fiber->status = FIBER_PENDING;
	fiber->context = attr.context;
	fiber->ss = attr.stack_size;
	fiber->priority = attr.priority;
	(fiber->priority_idx ? __updatepriority : __setpriority)(*fid);
}

void fiber_destroy(fid_t fid)
{
	fiber_t *fiber;

	fiber = __getfiber(fid);
	assert(fiber->status == FIBER_DONE);
#ifdef USE_CORO
	(void)coro_destroy(&fiber->coroutine);
	coro_stack_free(&fiber->stack);
#endif
	bzero(fiber, sizeof(fiber_t));
	free(fiber->coroutine_ctx);
	fiber->coroutine_ctx = NULL;
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
		free(__fibers.queue);
		bzero(&__fibers, sizeof(__fibers));
	}
}

void *fiber_call(fid_t fid, void *context)
{
	fiber_t *fiber;
	fiber_t *current;

	current = __getfiber(__fiber_current);
	if (current->caller == fid)
		return fiber_yield(context);
	fiber = __getfiber(fid);
	assert(__fiber_current != fid);
	assert(fiber->status == FIBER_PENDING);
	fiber->context = context;
	fiber->caller = __fiber_current;
	__fiber_current = fiber->fid;
	__fiber_idx = fiber->priority_idx;
	fiber->status = FIBER_RUNNING;
	if (current->status == FIBER_RUNNING)
		current->status = FIBER_PENDING;
#ifdef USE_PICORO
	fiber->result = resume(fiber->coroutine, fiber->coroutine_ctx);
#elif defined(OS_PROVENCORE)
	int dummy;

	if (resume(fiber->coroutine, &dummy))
		fiber->status = FIBER_DONE;
#else
	LOG_DEBUG("fiber call: %d > %d", current->fid, fiber->fid);
	coro_transfer(&current->coroutine, &fiber->coroutine);
#endif
	return fiber->result;
}

__always_inline bool fiber_isdone(fid_t fid)
{
	return __getfiber(fid)->status >= FIBER_DONE;
}

__always_inline void fiber_schedule(void)
{
	uint16_t begin;
	fiber_t *fiber;

	begin = __fiber_idx;
	if (__fiber_idx + 1 >= __fibers.len)
		__fiber_idx = 0;
	else
		++__fiber_idx;
	while ((fiber = __getfiber(__fibers.queue[__fiber_idx]))->status
		!= FIBER_PENDING) {
		if (begin == __fiber_idx)
			return;
		if (++__fiber_idx >= __fibers.len)
			__fiber_idx = 0;
	}
	fiber_call(fiber->fid, fiber->context);
}

__always_inline void fiber_join(fid_t fid)
{
	while (!fiber_isdone(fid))
		fiber_schedule();
}

void *fiber_yield(void *context)
{
	fiber_t *fiber;
	fiber_t *caller;

	fiber = __getfiber(__fiber_current);
	caller = __getfiber(fiber->caller);
	assert(fiber->status == FIBER_RUNNING
		|| fiber->status == FIBER_DONE
		|| fiber->status == FIBER_BLOCKING);
	assert(caller->status == FIBER_PENDING
		|| caller->status == FIBER_BLOCKING
		|| caller->status == FIBER_DONE);
	while (caller && caller->status != FIBER_PENDING)
		caller = __getfiber(caller->caller);
	assert(caller);
	assert(caller->status == FIBER_PENDING);
	__fiber_current = caller->fid;
	__fiber_idx = caller->priority_idx;
	fiber->caller = 0;
	if (fiber->status == FIBER_RUNNING)
		fiber->status = FIBER_PENDING;
	caller->status = FIBER_RUNNING;
#ifdef USE_PICORO
	return yield(context);
#else
	fiber->result = context;
# ifdef OS_PROVENCORE
	if (caller) {
		int dummy;

		if (resume(caller->coroutine, &dummy))
			caller->status = FIBER_DONE;
	} else {
		yield();
	}
# else
	LOG_DEBUG("fiber yield: %d > %d", fiber->fid, caller->fid);
	coro_transfer(&fiber->coroutine, &caller->coroutine);
# endif
	return fiber->context;
#endif
}

__always_inline fid_t fiber_getfid(void)
{
	return __fiber_current;
}

__always_inline void fiber_setstate(fiber_st_t st)
{
	fiber_t *fiber;

	fiber = __getfiber(__fiber_current);
	assert(fiber->status != st);
	fiber->status = st;
}

__always_inline void fiber_lock(void)
{
	fiber_t *fiber;

	fiber = __getfiber(__fiber_current);
	assert(fiber->status == FIBER_RUNNING);
	fiber->status = FIBER_BLOCKING;
	fiber_schedule();
}

__always_inline void fiber_unlock(fid_t fid)
{
	fiber_t *fiber;

	fiber = __getfiber(fid);
	fiber->status = FIBER_PENDING;
}
