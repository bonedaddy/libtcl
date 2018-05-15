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

#include <osi/sema.h>

int sema_init(sema_t *sema, unsigned value)
{
#if defined(OSI_THREAD_MOD)
	if ((sema->handle = eventfd(value, EFD_SEMAPHORE)) < 0)
		return sema->handle;
#else
	(void)value;
	sema->handle = 0;
#endif
	return 0;
}

void sema_destroy(sema_t *sema)
{
#if defined(OSI_THREAD_MOD)
	if (sema->handle >= 0)
    	close(sema->handle);
#else
	sema->handle = 0;
#endif
}

void sema_wait(sema_t *sema)
{
#if defined(OSI_THREAD_MOD)
	eventfd_t value;

	eventfd_read(sema->handle, &value);
#else
	--sema->handle;
#endif
}

bool sema_trywait(sema_t *sema)
{
#if defined(OSI_THREAD_MOD)
	int flags;
	bool rc;
	eventfd_t value;

	if ((flags = fcntl(sema->handle, F_GETFL)) < 0)
		return false;
	if (fcntl(sema->handle, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;
	rc = true;
	if (eventfd_read(sema->handle, &value))
    	rc = false;
	fcntl(sema->handle, F_SETFL, flags);

	return rc;
#else
	return --sema->handle == 0;
#endif
}

void sema_post(sema_t *sema)
{
#if defined(OSI_THREAD_MOD)
	eventfd_write(sema->handle, 1ULL);
#else
	++sema->handle;
#endif
}
