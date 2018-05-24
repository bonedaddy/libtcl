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

#define LOG_TAG "bt_osi_event"

#include "osi/event.h"
#include "osi/log.h"

int event_init(event_t *event, event_value_t value,
	unsigned flags)
{
#ifdef OSI_THREADING
	if ((event->fd = eventfd((unsigned)value, flags)) < 0)
		return -1;
#else
	if (flags & ~EVENT_FLAGS_SET) {
		errno = EINVAL;
		return -1;
	}
	event->count = value;
	event->flags = flags;
	queue_init(&event->wq, sizeof(fid_t));
#endif /* OSI_THREADING */
	return 0;
}

void event_destroy(event_t *event)
{
#ifdef OSI_THREADING
	if (event->fd >= 0)
		close(event->fd);
#else
	queue_destroy(&event->wq, NULL);
#endif /* OSI_THREADING */
}

bool event_tryread(event_t *event, event_value_t *value)
{
#ifdef OSI_THREADING
	int flags;
	bool rc;

	if ((flags = fcntl(event->fd, F_GETFL)) < 0)
		return false;
	if (fcntl(event->fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;
	rc = true;
	if (eventfd_read(event->fd, value) < 0)
    	rc = false;
	fcntl(event->fd, F_SETFL, flags);

	return rc;
#else
	event_value_t ucnt;

	if (!event->count)
		return false;
	ucnt = (event->flags & EVENT_SEMAPHORE) ? 1 : event->count;
	event->count -= ucnt;
	if (value) *value = ucnt;
	return true;
#endif /* OSI_THREADING */
}

int event_read(event_t *event, event_value_t *value)
{
#ifdef OSI_THREADING
	return eventfd_read(event->fd, value);
#else
	int res;
	event_value_t ucnt;
	fid_t current;

	if (event->count > 0) res = sizeof(ucnt);
	else {
		current = fiber_getfid();
		*(fid_t *)queue_push(&event->wq) = current;
		fiber_setstate(FIBER_BLOCKING);
		while (true) {
			if (event->count > 0) {
				res = sizeof(ucnt);
				break;
			}
			fiber_schedule();
		}
	}
	ucnt = (event->flags & EVENT_SEMAPHORE) ? 1 : event->count;
	if ((event->count -= ucnt)) {
		queue_pop(&event->wq, &current);
		fiber_unlock(current);
	}
	if (value) *value = ucnt;
	return res;
#endif /* OSI_THREADING */
}

int event_write(event_t *event, event_value_t value)
{
#ifdef OSI_THREADING
	return eventfd_write(event->fd, value);
#else
	event_value_t ucnt;
	fid_t current;

	ucnt = value;
	if (ucnt == ULLONG_MAX) {
		errno = EINVAL;
		return -1;
	}
	if (ULLONG_MAX - event->count <= ucnt) {
		current = fiber_getfid();
		*(fid_t *)queue_push(&event->wq) = current;
		fiber_setstate(FIBER_BLOCKING);
		while (true) {
			if (ULLONG_MAX - event->count > ucnt)
				break;
			fiber_schedule();
		}
	}
	event->count += ucnt;
	queue_pop(&event->wq, &current);
	fiber_unlock(current);
	return 0;
#endif /* OSI_THREADING */
}
