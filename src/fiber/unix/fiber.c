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

#include "internal.h"

#ifdef FIBER_UNIX

OSI_STACK_IMPL(, osi_fibers, osi_fiber_t, uint16_t)

# ifdef HAS_UCONTEXT_H
#   include <ucontext.h>

int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss)
{
	bzero(fiber, sizeof(osi_fiber_t));
	fiber->stack = calloc(ss, sizeof(char));
	getcontext(&fiber->context);
	fiber->context.uc_stack.ss_sp = fiber->stack;
	fiber->context.uc_stack.ss_size = ss;
	fiber->context.uc_link = NULL;
	makecontext(&fiber->context, (void (*)())fn, 1, fiber);
	return 0;
}

int osi_fiber_dtor(osi_fiber_t *fiber)
{
	free(fiber->stack);
	return 0;
}

# endif /* HAS_UCONTEXT_H */

#endif /* FIBER_UNIX */
