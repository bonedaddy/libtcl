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

#include "fiber.h"

#ifdef FIBER_NATIVE
#include <ucontext.h>

struct osi_fiber
{
	void *stack;
	ucontext_t context;
};

static osi_fiber_t main_fiber = { };
static osi_fiber_t *current_fiber = &main_fiber;

int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss)
{
	bzero(fiber, sizeof(osi_fiber_t));
	fiber->stack = calloc(ss, sizeof(char));
	getcontext(&fiber->context);
	fiber->context.uc_stack.ss_sp = fiber->stack;
	fiber->context.uc_stack.ss_size = ss;
	fiber->context.uc_link = NULL;
	makecontext(&fiber->context, (void (*)()) fn, 1, fiber);
	return 0;
}

int osi_fiber_dtor(osi_fiber_t *fiber)
{
	assert(fiber != current_fiber);
	assert(fiber != &main_fiber);

	free(fiber->stack);
	return 0;
}

void osi_fiber_switch(osi_fiber_t *fiber)
{
	ucontext_t *current_context;

	current_context = &current_fiber->context;
	current_fiber = fiber;
	swapcontext(current_context, &fiber->context);
}
#endif
