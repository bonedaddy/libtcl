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

#include "sched.h"

#ifdef FIBER_UNIX
# include "sched.h"

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

# ifdef HAS_UCONTEXT_H
#   include <ucontext.h>

/*static void sched_switch(osi_fiber_t *fiber)
{
	ucontext_t *current_context;

	current_context = &current_fiber->context;
	current_fiber = fiber;
	swapcontext(current_context, &fiber->context);
}*/

# endif /* HAS_UCONTEXT_H */

#endif /* FIBER_UNIX */
