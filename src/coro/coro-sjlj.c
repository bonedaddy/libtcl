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

#include "coro/internal.h"

#include <setjmp.h>

static struct coro {
	struct coro *next;
	jmp_buf state;
} __main, *__self = &__main, *__idle;

static int __resumable(coro_t c)
{
	return c != NULL && c->next == NULL;
}

static void __push(coro_t *list, coro_t c)
{
	c->next = *list;
	*list = c;
}

static coro_t __pop(coro_t *list)
{
	coro_t c;
	
	c = *list;
	*list = c->next;
	c->next = NULL;
	return c;
}

NOINLINE
static void *__pass(coro_t me, void *arg)
{
	static void *saved;

	saved = arg;
	if(!setjmp(me->state))
		longjmp(__self->state, 1);
	return saved;
}

static void __start(void);

NOINLINE
static void __root(void *ret)
{
	fn_t *fn;
	struct coro me;

	__push(&__idle, &me);
	fn = __pass(&me, ret);
	if (!setjmp(__self->state))
		__start();
	while (true) {
		ret = fn(coro_yield(&me));
		__push(&__idle, __pop(&__self));
		fn = __pass(&me, ret);
	}
}

NOINLINE
static void __start(void)
{
	char stack[16 * 1024];
	__root(stack);
}

NOINLINE
int coro_init(coro_t *coro, fn_t *fn)
{
	if (__idle == NULL && !setjmp(__self->state))
		__start();
	*coro = coro_resume(__pop(&__idle), fn);
	return 0;
}

void *coro_resume(coro_t c, void *arg)
{
	assert(__resumable(c));
	__push(&__self, c);
	return(__pass(c->next, arg));
}

void *coro_yield(void *arg)
{
	return __pass(__pop(&__self), arg);
}

FORCEINLINE
coro_t coro_self(void)
{
	return __self;
}
