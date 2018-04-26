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

static osi_fiber_t main_fiber = { };
static osi_fiber_t *current_fiber = &main_fiber;
static osi_fibers_t pending = { };

void osi_sched_push(osi_fiber_t *fiber)
{
	memcpy(osi_fibers_push(&pending, 1), fiber, sizeof(osi_fiber_t));
}

osi_fiber_t	*osi_sched_current(void)
{
	return current_fiber;
}

void osi_sched_switch(osi_fiber_t *fiber)
{
	ucontext_t *ucontext;

	if (fiber != current_fiber) {
		ucontext = &current_fiber->context;
		current_fiber = fiber;
		swapcontext(ucontext, &fiber->context);
	}
}

int osi_yield(void)
{
	osi_fiber_t *fiber;

	if (osi_fibers_len(&pending)) {
		fiber = osi_fibers_peek(&pending);
		osi_fibers_pop(&pending, 1);
	} else {

	}
	osi_sched_switch(fiber);
}
