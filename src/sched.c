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

#include <osi/sched.h>
#include <osi/string.h>

#include <errno.h>

static sched_t __sched = {
	.pool = {
		NULL, 0, 0,
		{ (head_t *)&__sched.pool.ready, (head_t *)&__sched.pool.ready, 0 },
		{ (head_t *)&__sched.pool.dead, (head_t *)&__sched.pool.dead, 0 },
	},
	.scheduled = false
};
sched_t *stdsched = &__sched;

void sched_init(sched_t *sched)
{
	sched->scheduled = false;
	fiber_pool_init(&sched->pool);
}

static fiber_t *__spawn(sched_t *sched, work_t *work, uint16_t ss, void *arg,
	int prio, uint8_t flags)
{
	fiber_t *fib;

	fib = fiber_pool_new(&sched->pool);
	if (!(fib->status == FIBER_EXITING))
		fiber_init(fib, work, ss, flags);
	else {
#ifdef USE_CORO
		if (fib->stack.ssze < ss) {
			coro_stack_free(&fib->stack);
			coro_stack_alloc(&fib->stack, ss);
		}
#endif
		fiber_reuse(fib, work, flags);
	}
	fib->arg = arg;
	fib->priority = prio;
	fiber_pool_ready(&sched->pool, fib);
	return fib;
}

fiber_t *sched_spawn(sched_t *sched, work_t *work, uint16_t ss, void *arg,
	int prio)
{
	return __spawn(sched, work, ss, arg, prio, FIBER_NONE);
}

fiber_t *sched_loop(sched_t *sched, work_t *work, uint16_t ss, void *arg,
	int prio)
{
	return __spawn(sched, work, ss, arg, prio, FIBER_LOOP);
}

void sched_start(sched_t *sched)
{
	fiber_t *fiber;

	if (sched->scheduled) {
		errno = EINVAL;
		return;
	}
	sched->scheduled = true;

	/* Schedule ready fibers */
	while (sched->scheduled) {
		if (!(fiber = fiber_pool_pop(&sched->pool)))
			break;
		fiber->status = FIBER_RUNNING;
		fiber_call(fiber, fiber->arg);
		if (fiber->status == FIBER_EXITING)
			fiber_pool_dead(&sched->pool, fiber);
		else
			fiber_pool_ready(&sched->pool, fiber);
	}

	fiber_pool_destroy(&sched->pool);
	sched->scheduled = false;
}

void sched_stop(sched_t *sched)
{
	sched->scheduled = false;
}
