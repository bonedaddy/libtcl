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

#pragma once

/*!@file coro/internal.h
 * @author uael
 */
#ifndef __CORO_INTERNAL_H
# define __CORO_INTERNAL_H

#include "osi/coroutine.h"

#ifndef CORO_STACK_SIZE
# define CORO_STACK_SIZE (4096UL)
#endif

#if defined(CORO_X86_64) \
	|| defined (CORO_X86_32) \
	|| defined (CORO_ARM) \
	|| defined (CORO_UCONTEXT)

# ifndef CORO_POOL_SIZE
#   define CORO_POOL_SIZE (100UL)
# endif

__ext coro_t __corolocate(void *ptr);
__ext coro_t __coroalloc(void);
__ext void __corofree(coro_t coro);

__ext void __coromake(coro_t from, uint16_t stack_size, fn_t *fn);
__ext void __coroswitch(coro_t from, coro_t to);
__ext void __coromain(fn_t *fn);

struct coro {

# ifdef CORO_UCONTEXT

	/*! Unix ucontext. */
	ucontext_t ctx;
# else

	/*! Stack origin pointer. */
	void *sp;
# endif

	/*! Coroutine linked list. */
	coro_t prev, next;

	/*! Last ret. */
	void *ret;

	unsigned int flag;
};

#endif

#endif /* !__CORO_INTERNAL_H */
