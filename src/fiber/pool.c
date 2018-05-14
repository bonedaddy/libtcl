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

#include <osi/fiber/pool.h>
#include <osi/string.h>

void fiber_pool_init(fiber_pool_t *pool)
{
	bzero(pool, sizeof(fiber_pool_t));
	list_init(&pool->ready);
	list_init(&pool->dead);
}

void fiber_pool_destroy(fiber_pool_t *pool)
{
	node_t *head;
	fiber_t *fib;

	/* Release ready fibers */
	while ((head = list_shift(&pool->ready)) != NULL) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		fiber_destroy(fib);
	}

	/* Release dead fibers */
	while ((head = list_shift(&pool->dead)) != NULL) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		fiber_destroy(fib);
	}

	free(pool->fibers);
}

fiber_t *fiber_pool_new(fiber_pool_t *pool)
{
	node_t *head;
	fiber_t *fiber;

	if ((head = list_pop(&pool->dead)))
		fiber = LIST_ENTRY(head, fiber_t, hold);
	else {
		size_t size;

		if (pool->size < pool->slot + 1) {
			size = pool->size ? pool->size * 2 : 32;
			pool->fibers = realloc(pool->fibers, size * sizeof(fiber_t));
			bzero(pool->fibers + (pool->size * sizeof(fiber_t)),
				(size - pool->size) * sizeof(fiber_t));
			pool->size = size;
		}
		fiber = pool->fibers + pool->slot++;
	}
	return fiber;
}

void fiber_pool_ready(fiber_pool_t *pool, fiber_t *fiber)
{
	node_t *head;
	node_t *entry;
	fiber_t *fib;

	fiber->status = OSI_FIB_READY;
	head = pool->ready.succ;
	entry = &fiber->hold;
	while (head != (node_t *)&pool->ready) {
		fib = LIST_ENTRY(head, fiber_t, hold);
		if (fiber->priority > fib->priority) {
			entry->pred = head->pred;
			entry->succ = head;
			head->pred->succ = entry;
			head->pred = entry;
			pool->ready.len++;
			return;
		}
		head = head->succ;
	}
	list_unshift(&pool->ready, entry);

}

void fiber_pool_dead(fiber_pool_t *pool, fiber_t *fiber)
{
	fiber->status = OSI_FIB_EXITING;
	list_unshift(&pool->dead, &fiber->hold);
}

fiber_t *fiber_pool_pop(fiber_pool_t *pool)
{
	node_t *head;

	if ((head = list_shift(&pool->ready)))
		return LIST_ENTRY(head, fiber_t, hold);
	return NULL;
}
