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

#if defined(HAS_SYS_EVENTFD_H)
# include <sys/eventfd.h>
# include <sys/fcntl.h>
#endif

struct sema {
	int handle;
};

sema_t *sema_new(unsigned value)
{
	sema_t *sema;

	sema = malloc(sizeof(sema_t));
#if defined(HAS_SYS_EVENTFD_H)
	sema->handle = eventfd(value, EFD_SEMAPHORE);
	if (sema->handle < 0) {
		free(sema);
		sema = NULL;
	}
#else
	(void)value;
	sema->handle = 0;
#endif
	return sema;
}

void sema_del(sema_t *sema)
{
#if defined(HAS_SYS_EVENTFD_H)
	if (sema->handle >= 0)
    	close(sema->handle);
#else
	sema->handle = 0;
#endif
	free(sema);
}

void sema_wait(sema_t *sema)
{
#if defined(HAS_SYS_EVENTFD_H)
	eventfd_t value;

	eventfd_read(sema->handle, &value);
#else
	--sema->handle;
#endif
}

bool sema_trywait(sema_t *sema)
{
#if defined(HAS_SYS_EVENTFD_H)
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
#if defined(HAS_SYS_EVENTFD_H)
	eventfd_write(sema->handle, 1ULL);
#else
	++sema->handle;
#endif
}
