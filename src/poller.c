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

#define LOG_TAG "bt_osi_poller"

#include "osi/poller.h"
#include "osi/log.h"

int poller_init(poller_t *poller, unsigned size)
{
#ifdef OSI_THREADING
	if ((poller->fd = epoll_create((int)size)) < 0)
		return -1;
#else
	poller->size = size;
	map_init(&poller->events, NULL, NULL);
#endif
	return 0;
}

void poller_destroy(poller_t *poller)
{
#ifdef OSI_THREADING
	close(poller->fd);
#else
	map_destroy(&poller->events, free);
#endif
}

int poller_add(poller_t *poller, event_t *ev, pollev_t attr)
{
#ifdef OSI_THREADING
	struct epoll_event event;

	event.events = attr.events;
	event.data.ptr = attr.ptr;
	return epoll_ctl(poller->fd, EPOLL_CTL_ADD, ev->fd, &event);
#else
	pollev_t *event;

	if (map_contains(&poller->events, ev))
		return 1;
	event = malloc(sizeof(pollev_t));
	memcpy(event, &attr, sizeof(pollev_t));
	map_set(&poller->events, ev, event);
	return 0;
#endif
}

int poller_del(poller_t *poller, event_t *ev)
{
#ifdef OSI_THREADING
	return epoll_ctl(poller->fd, EPOLL_CTL_DEL, ev->fd, NULL);
#else
	pollev_t *event;

	if ((event = map_get(&poller->events, ev))) {
		free(event);
		map_remove(&poller->events, ev);
	}
	return 0;
#endif
}

int poller_wait(poller_t *poller, pollev_t *events, int size)
{
#ifdef OSI_THREADING
	int ret, i;
	struct epoll_event epoll_events[size];

	do (ret = epoll_wait(poller->fd, epoll_events, size, -1));
	while (ret < 0 && errno == EINTR);

	for (i = 0; i < ret; ++i) {
		events[i].events = 0;
		if (epoll_events[i].events &
			(EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR))
			events[i].events |= POLLER_IN;
		if (epoll_events[i].events & EPOLLOUT)
			events[i].events |= POLLER_OUT;
		events[i].ptr = epoll_events[i].data.ptr;
	}
	
#else
	int ret;
	size_t it;
	const event_t *ev;
	const pollev_t *event;

	ret = 0;
	while (poller->events.length && ret < size) {
		for (it = 0; it < poller->events.capacity; ++it) {
			/* TODO(uael): ops */
			if (!(poller->events.flags[it] & 0b11111111)) {
				ev = poller->events.keys[it];
				event = poller->events.values[it];

				/* TODO(uael): poller out */
				if (ev->count) {
					events[ret].events = POLLER_IN;
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
