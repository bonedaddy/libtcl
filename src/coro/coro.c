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

int coro_init(coro_t *coro, fn_t *fn, size_t stack_size)
{
	if (!(*coro = __coroalloc(stack_size)))
		return -1;

	__coromake(*coro, fn);

	return 0;
}

void *coro_resume(coro_t *coro, void *arg)
{
	coro_t cur;

	cur = __self;

	(*coro)->caller = cur;
	(*coro)->ret = arg;

	__self = (*coro);
	__coroswitch(cur, (*coro));

	if ((*coro)->flag & CORO_FLAG_END) {
		__cororelease(*coro);
		*coro = NULL;
	}

	return cur->ret;
}

void* coro_yield(void *arg)
{
	coro_t to, cur = NULL;

	cur = __self;

	to = cur->caller;
	cur->caller = NULL;
	to->ret = arg;

	__self = to;
	__coroswitch(cur, to);

	return cur->ret;
}

NOINLINE REGPARAM(0)
void __coromain(fn_t *fn)
{
	void *arg, *ret;

	arg = __self->ret;
	ret = fn(arg);
	__self->flag = CORO_FLAG_END;
	coro_yield(ret);
}

void coro_destroy(coro_t *coro)
{
	if (*coro) {
		__cororelease(*coro);
		*coro = NULL;
	}
}

coro_t coro_self(void)
{
	return __self;
}
