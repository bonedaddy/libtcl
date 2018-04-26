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

#include "../internal.h"
#include "../event.h"
#include "../sched.h"

static void fiber_func(void *ctx)
{
	osi_fiber_t *fiber;

	fiber = (osi_fiber_t *)ctx;
	fiber->fn(fiber->ctx);
	osi_event_set(&fiber->stop_ev);
	osi_yield();
}

int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss)
{
	if (!ss)
		ss = (uint16_t)DEFAULT_FIBER_STACK_SIZE;
	bzero(fiber, sizeof(osi_fiber_t));
	fiber->fn = fn;
	osi_event_ctor(&fiber->stop_ev, OSI_EVENT_MANUAL);
	fiber->stack = calloc(ss, sizeof(char));
	getcontext(&fiber->context);
	fiber->context.uc_stack.ss_sp = fiber->stack;
	fiber->context.uc_stack.ss_size = ss;
	fiber->context.uc_link = NULL;
	makecontext(&fiber->context, (void (*)(void))fiber_func, 1, fiber);
	return 0;
}

int osi_fiber_dtor(osi_fiber_t *fiber)
{
	osi_event_dtor(&fiber->stop_ev);
	free(fiber->stack);
	return 0;
}

int osi_fiber_start(osi_fiber_t *fiber, void *ctx)
{
	fiber->ctx = ctx;
	osi_sched_push(fiber);
	return 0;
}

int osi_fiber_join(osi_fiber_t *fiber)
{
	osi_event_wait(&fiber->stop_ev);
	return 0;
}
