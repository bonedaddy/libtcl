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

#include "osi/sema.h"
#include "osi/fiber.h"

int sema_init(sema_t *sema, unsigned value)
{
#ifdef OSI_THREADING
	if ((sema->handle = eventfd(value, EFD_SEMAPHORE)) < 0)
		return sema->handle;
#else
	sema->handle = value;
	list_init(&sema->queue);
#endif /* OSI_THREADING */
	return 0;
}

void sema_destroy(sema_t *sema)
{
#ifdef OSI_THREADING
	if (sema->handle >= 0)
    	close(sema->handle);
#else
	list_destroy(&sema->queue, NULL);
#endif /* OSI_THREADING */
	sema->handle = INVALID_FD;
}

void sema_wait(sema_t *sema)
{
#ifdef OSI_THREADING
	eventfd_t value;

	eventfd_read(sema->handle, &value);
#else
	head_t *head;
	fiber_t *fiber;

	while (!sema->handle) {
		fiber = fiber_current();
		list_push(&sema->queue, &fiber->hold);
		fiber_yield(NULL);
	}
	if (--sema->handle > 0 && (head = list_shift(&sema->queue))) {
		fiber = LIST_ENTRY(head, fiber_t, hold);
		if (!fiber_isdone(fiber))
			fiber_call(fiber, NULL);
	}
#endif /* OSI_THREADING */
}

bool sema_trywait(sema_t *sema)
{
#ifdef OSI_THREADING
	int flags;
	bool rc;
	eventfd_t value;

	if ((flags = fcntl(sema->handle, F_GETFL)) < 0)
		return false;
	if (fcntl(sema->handle, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;
	rc = true;
	if (eventfd_read(sema->handle, &value) < 0)
    	rc = false;
	fcntl(sema->handle, F_SETFL, flags);

	return rc;
#else
	head_t *head;
	fiber_t *fiber;

	if (!sema->handle) {
		fiber = fiber_current();
		list_push(&sema->queue, &fiber->hold);
		fiber_yield(NULL);
	}
	if (!sema->handle)
		return false;
	if (--sema->handle > 0 && (head = list_shift(&sema->queue))) {
		fiber = LIST_ENTRY(head, fiber_t, hold);
		if (!fiber_isdone(fiber))
			fiber_call(fiber, NULL);
	}
	return true;
#endif /* OSI_THREADING */
}

void sema_post(sema_t *sema)
{
#ifdef OSI_THREADING
	eventfd_write(sema->handle, 1ULL);
#else
	head_t *head;
	fiber_t *fiber;

	if (++sema->handle == 1 && (head = list_shift(&sema->queue))) {
		fiber = LIST_ENTRY(head, fiber_t, hold);
		if (!fiber_isdone(fiber))
			fiber_call(fiber, NULL);
	}
#endif /* OSI_THREADING */
}
