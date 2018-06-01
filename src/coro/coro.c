/*
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

#include <stdio.h>

#include "coro/internal.h"
#include "osi/string.h"

#define CORO_FLAG_END 0x00000001

static struct coro __main, *__self = &__main;

void *coro_resume(coro_t coro, void *arg)
{
	struct coro *cur, *to = coro;

	cur = __self;

	cur->next = to;
	to->prev = cur;
	to->ret = arg;

	__self = to;
	__coroswitch(cur, to);

	if (to->flag & CORO_FLAG_END) {
		memset(to, 0, sizeof(struct coro));
		/* give a chance to avoid dead-locking */
		__corofree(to);
	}

	return cur->ret;
}

NOINLINE
static void *do_coro_yield(void *arg, unsigned int flag)
{
	struct coro *to, *cur = NULL;

	cur = __self;
	to = cur->prev;
	cur->prev = NULL;
	to->next = NULL;
	to->ret = arg;
	cur->flag = flag;
	__self = to;
	__coroswitch(cur, to);
	return cur->ret;
}

void* coro_yield(void *arg)
{
	return do_coro_yield(arg, 0);
}

NOINLINE REGPARAM(0)
void __coromain(void *(*f)(void *))
{
	struct coro *cur;
	void *arg, *ret;

	cur = __self;
	arg = cur->ret;
	ret = f(arg);
	do_coro_yield(ret, CORO_FLAG_END);
}

int coro_init(coro_t *coro, void *(*f)(void*))
{
	struct coro *ctx;

	if (!(ctx = __coroalloc()))
		return -1;
	memset(ctx, 0, sizeof(struct coro));

	__coromake(ctx, CORO_STACK_SIZE, f);

	*coro = ctx;
	return 0;
}

void coro_destroy(coro_t coro)
{
	(void)coro;
}

coro_t coro_self(void)
{
	return __self;
}
