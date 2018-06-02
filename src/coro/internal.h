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

__ext coro_t __coroalloc(size_t stack_size);
__ext void __cororelease(coro_t coro);

__ext void __coromake(coro_t from, fn_t *fn);
__ext void __coroswitch(coro_t from, coro_t to);
__ext void __coromain(fn_t *fn);

struct coro {

# ifdef CORO_UCONTEXT

	/*! Unix ucontext. */
	ucontext_t ctx;
# else

	/*! Stack pointer origin. */
	void *sp;
# endif

	/*! Stack size. */
	size_t ssize;

	/*! Coroutine linked list. */
	coro_t caller;

	/*! Last ret. */
	void *ret;

	unsigned int flag;

#ifdef RUNNING_ON_VALGRIND
	int valgrind_id;
#endif
};

#endif

#endif /* !__CORO_INTERNAL_H */
