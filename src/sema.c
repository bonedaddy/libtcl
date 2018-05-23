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
	queue_init(&sema->queue, sizeof(fid_t));
#endif /* OSI_THREADING */
	return 0;
}

void sema_destroy(sema_t *sema)
{
#ifdef OSI_THREADING
	if (sema->handle >= 0)
    	close(sema->handle);
#else
	queue_destroy(&sema->queue, NULL);
#endif /* OSI_THREADING */
	sema->handle = INVALID_FD;
}

void sema_wait(sema_t *sema)
{
#ifdef OSI_THREADING
	eventfd_t value;

	eventfd_read(sema->handle, &value);
#else
	fid_t fid;

	if (!sema->handle) {
		*(fid_t *)queue_push(&sema->queue) = fiber_getfid();
		fiber_lock();
		assert(sema->handle);
	}
	if (--sema->handle > 0 && queue_pop(&sema->queue, &fid))
		fiber_unlock(fid);
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
	fid_t fid;

	if (!sema->handle)
		return false;
	if (--sema->handle > 0 && queue_pop(&sema->queue, &fid))
		fiber_unlock(fid);
	return true;
#endif /* OSI_THREADING */
}

void sema_post(sema_t *sema)
{
#ifdef OSI_THREADING
	eventfd_write(sema->handle, 1ULL);
#else
	fid_t fid;

	if (++sema->handle == 1 && queue_pop(&sema->queue, &fid))
		fiber_unlock(fid);
#endif /* OSI_THREADING */
}
