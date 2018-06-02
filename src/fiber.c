/*
 * Copyright (C) 2014 Google, Inc.
 * Copyright (C) 2018 Tempow
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
#include "osi/vector.h"

#ifndef FIBER_MAX
# define FIBER_MAX 256
#endif

enum {

	/*! The fiber was just created so ready */
	FIBER_ACTIVE,

	/*! TODO */
	FIBER_BLOCKING,

	/*! The fiber is terminated but still exists */
	FIBER_DONE
};

struct fiber {

	uint16_t id;

	int8_t priority;

	uint8_t state;

	void *arg;

	void *ret;

	coro_t coroutine;

	struct fiber *next;
};

struct {

	struct fiber fibers[FIBER_MAX];

	struct fiber *head, *self;

	uint16_t fibers_idx;

	fiber_attr_t dft_attr;

} __scope;

static void __initmain(void)
{
	static int init;
	struct fiber *head;

	if (init)
		return;
	init = 1;

	head = __scope.fibers + __scope.fibers_idx;
	head->id = __scope.fibers_idx++;

	head->coroutine = coro_self();
	head->state = FIBER_ACTIVE;
	head->priority = 0;
	head->arg = NULL;
	head->ret = NULL;
	head->next = NULL;

	coro_setdata(coro_self(), (void *)(uintptr_t)head->id);

	__scope.head = head;
}

int fiber_create(fiber_t *fiber, const fiber_attr_t *attr,
				 routine_t *routine, void *arg)
{
	struct fiber *node, *head;

	if (__scope.fibers_idx >= FIBER_MAX) {
		errno = EINVAL;
		return -1;
	}

	if (LIKELY(!attr))
		attr = &__scope.dft_attr;

	if (coro_init(fiber, routine, attr->stack_size))
		return -1;

	__initmain();

	node = __scope.fibers + __scope.fibers_idx;
	node->id = __scope.fibers_idx++;

	node->coroutine = *fiber;
	node->state = FIBER_ACTIVE;
	node->priority = attr->prio;
	node->arg = arg;
	node->ret = NULL;
	node->next = NULL;

	coro_setdata(*fiber, (void *)(uintptr_t)node->id);

	if (!__scope.head || __scope.head->priority >= node->priority) {
		node->next = __scope.head;
		__scope.head = node;
	} else {
		head = __scope.head;
		while (head->next
			&& head->next->priority < node->priority) {
			head = head->next;
		}
		node->next = head->next;
		head->next = node;
	}

	return 0;
}

int fiber_cancel(fiber_t fiber)
{
	struct fiber *data;

	if (!fiber) {
		errno = EINVAL;
		return -1;
	}

	data = __scope.fibers + (uint16_t)(uintptr_t)coro_getdata(fiber);
	if (data->state == FIBER_DONE) {
		errno = EINVAL;
		return -1;
	}

	if (fiber == fiber_self())
		fiber_exit(NULL);
	else {
		data->state = FIBER_DONE;
		coro_kill(&data->coroutine);
	}

	return 0;
}

int fiber_join(fiber_t fiber, void **retval)
{
	struct fiber *data;

	if (!fiber) {
		errno = EINVAL;
		return -1;
	}

	data = __scope.fibers + (uint16_t)(uintptr_t)coro_getdata(fiber);
	if (data->state == FIBER_DONE) {
		errno = EINVAL;
		return -1;
	}

	while (data->state != FIBER_DONE)
		fiber_yield();

	if (retval)
		*retval = data->ret;

	return 0;
}

int fiber_setschedprio(fiber_t fiber, int prio)
{
	(void)fiber;
	(void)prio;
	/* TODO(tempow): update priority */
	return 0;
}

FORCEINLINE
void fiber_exit(void *retval)
{
	assert(__scope.self);

	__scope.self->state = FIBER_DONE;
	coro_exit(__scope.self->ret = retval);
}

FORCEINLINE
fiber_t fiber_self(void)
{
	return coro_self();
}

void fiber_lock(waitq_t *wqueue)
{
	struct fiber *data;

	data = __scope.fibers + (uint16_t)(uintptr_t)coro_getdata(fiber_self());
	data->state = FIBER_BLOCKING;
	waitq_push(wqueue, fiber_self());
}

void fiber_unlock(waitq_t *wqueue)
{
	fiber_t fiber;
	struct fiber *data;

	if ((fiber = waitq_pop(wqueue))) {
		data = __scope.fibers + (uint16_t)(uintptr_t)coro_getdata(fiber);
		data->state = FIBER_ACTIVE;
	}
}

void fiber_yield(void)
{
	struct fiber *begin;

	begin = __scope.self;

	if (!__scope.self || !__scope.self->next)
		__scope.self = __scope.head;
	else
		__scope.self = __scope.self->next;

	assert(__scope.self);

	while (__scope.self->state != FIBER_ACTIVE) {
		if (__scope.self->next == begin)
			return;
		__scope.self = __scope.self->next ? __scope.self->next : __scope.head;
	}
	__scope.self->ret =
		coro_resume(&__scope.self->coroutine, __scope.self->arg);
	if (!__scope.self->coroutine) {
		__scope.self->state = FIBER_DONE;
	}
}
