/*
 * Copyright (C) 2014 Google, Inc.
 * Copyright (C) 2018 Tempow
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

#define LOG_TAG "bt_osi_poll"

#include "osi/poll.h"
#include "osi/log.h"

int poll_init(poll_t *poll, size_t size)
{
#ifdef OSI_THREADING
	if ((poll->fd = epoll_create(size)) < 0)
		return -1;
#else
	poll->size = size;
	map_init(&poll->events, NULL, NULL);
#endif
	return 0;
}

void poll_destroy(poll_t *poll)
{
#ifdef OSI_THREADING
	close(poll->fd);
#else
	map_destroy(&poll->events, free);
#endif
}

int poll_add(poll_t *poll, event_t *ev, pollev_t attr)
{
#ifdef OSI_THREADING
	struct epoll_event event;

	event.events = attr.events;
	event.data.ptr = attr.ptr;
	return epoll_ctl(poll->fd, EPOLL_CTL_ADD, ev->fd, &event);
#else
	pollev_t *event;

	if (map_contains(&poll->events, ev))
		return 1;
	event = malloc(sizeof(pollev_t));
	memcpy(event, &attr, sizeof(pollev_t));
	map_set(&poll->events, ev, event);
	return 0;
#endif
}

int poll_del(poll_t *poll, event_t *ev)
{
#ifdef OSI_THREADING
	return epoll_ctl(poll->fd, EPOLL_CTL_DEL, ev->fd, NULL);
#else
	pollev_t *event;

	if ((event = map_get(&poll->events, ev))) {
		free(event);
		map_remove(&poll->events, ev);
	}
	return 0;
#endif
}

int poll_wait(poll_t *poll, pollev_t *events, int size)
{
#ifdef OSI_THREADING
	int ret, i;
	struct epoll_event epoll_events[size];

	do (ret = epoll_wait(poll->fd, epoll_events, size, -1));
	while (ret < 0 && errno == EINTR);

	for (i = 0; i < ret; ++i) {
		events[i].events = 0;
		if (epoll_events[i].events &
			(EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR))
			events[i].events |= POLL_IN_;
		if (epoll_events[i].events & EPOLLOUT)
			events[i].events |= POLL_OUT_;
		events[i].ptr = epoll_events[i].data.ptr;
	}
	
#else
	int ret;
	size_t it;
	const event_t *ev;
	const pollev_t *event;

	ret = 0;
	while (poll->events.length && ret < size) {
		for (it = 0; it < poll->events.capacity; ++it) {
			/* TODO(uael): ops */
			if (!(poll->events.flags[it] & 0b11111111)) {
				ev = poll->events.keys[it];
				event = poll->events.values[it];

				/* TODO(uael): poll out */
				if (ev->count) {
					events[ret].events = POLL_IN_;
					events[ret].ptr = event->ptr;
					++ret;
				}
			}
		}
		if (ret)
			break;
		fiber_schedule();
	}
#endif
	return ret;
}
