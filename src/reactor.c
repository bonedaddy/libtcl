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

#define LOG_TAG "osi_reactor"

#include "osi/reactor.h"
#include "osi/string.h"
#include "osi/log.h"

#ifdef OSI_THREADING
# ifndef EFD_SEMAPHORE
#   define EFD_SEMAPHORE (1 << 0)
# endif /* EFD_SEMAPHORE */

static reactor_st_t __run_reactor(reactor_t *reactor, int iterations);
static const size_t __max_events = 64;
static const eventfd_t __event_reactor_stop = 1;

int reactor_init(reactor_t *reactor)
{
	struct epoll_event event;

	if ((reactor->epoll_fd = epoll_create(__max_events)) < 0) {
		LOG_ERROR("unable to create epoll instance: %m");
		return -1;
	}
	if ((reactor->event_fd = eventfd(0, 0)) < 0) {
		LOG_ERROR("unable to create eventfd: %m");
		return -1;
	}
	bzero(&event, sizeof(event));
	event.events = EPOLLIN;
	event.data.ptr = NULL;
	if (epoll_ctl(reactor->epoll_fd, EPOLL_CTL_ADD, reactor->event_fd,
		&event) < 0) {
		LOG_ERROR("unable to register eventfd with epoll set: %m");
		return -1;
	}
	pthread_mutex_init(&reactor->invalidation_lock, NULL);
	set_init(&reactor->invalidation_set, NULL, NULL);
	return 0;
}

void reactor_destroy(reactor_t *reactor)
{
	set_destroy(&reactor->invalidation_set);
	close(reactor->event_fd);
	reactor->event_fd = INVALID_FD;
	close(reactor->epoll_fd);
	reactor->epoll_fd = INVALID_FD;
	pthread_mutex_destroy(&reactor->invalidation_lock);
}

reactor_st_t reactor_start(reactor_t *reactor)
{
	return __run_reactor(reactor, 0);
}

reactor_st_t reactor_run_once(reactor_t *reactor)
{
	return __run_reactor(reactor, 1);
}

void reactor_stop(reactor_t *reactor)
{
	eventfd_write(reactor->event_fd, __event_reactor_stop);
}

reactor_object_t *reactor_register(reactor_t *reactor, int fd, void *context,
	reactor_ready_t *read_ready, reactor_ready_t *write_ready)
{
	reactor_object_t *object;
	struct epoll_event event;

	object = (reactor_object_t *)malloc(sizeof(reactor_object_t));
	object->reactor = reactor;
	object->fd = fd;
	object->context = context;
	object->read_ready = read_ready;
	object->write_ready = write_ready;
	bzero(&event, sizeof(event));
	if (read_ready) event.events |= (EPOLLIN | EPOLLRDHUP);
	if (write_ready) event.events |= EPOLLOUT;
	event.data.ptr = object;
	if (epoll_ctl(reactor->epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		LOG_ERROR("unable to register fd %d to epoll set: %m", fd);
		pthread_mutex_destroy(&object->lock);
		free(object);
		return NULL;
	}
	pthread_mutex_init(&object->lock, NULL);
	return object;
}

void reactor_unregister(reactor_object_t *obj)
{
	reactor_t *reactor = obj->reactor;

	if (epoll_ctl(reactor->epoll_fd, EPOLL_CTL_DEL, obj->fd, NULL) < 0) {
		LOG_ERROR("unable to unregister fd %d from epoll set: %m", obj->fd);
	}
	if (reactor->is_running
		&& pthread_equal(pthread_self(), reactor->run_thread)) {
		reactor->object_removed = true;
		return;
	}
	pthread_mutex_lock(&reactor->invalidation_lock);
	set_put(&reactor->invalidation_set, obj);
	pthread_mutex_unlock(&reactor->invalidation_lock);

	pthread_mutex_lock(&obj->lock);
	pthread_mutex_unlock(&obj->lock);
	pthread_mutex_destroy(&obj->lock);
	free(obj);
}

static reactor_st_t __run_reactor(reactor_t *reactor, int iterations)
{
	int ret, i, j;
	reactor_object_t *object;
	struct epoll_event events[__max_events];

	reactor->run_thread = pthread_self();
	reactor->is_running = true;
	for (i = 0; iterations == 0 || i < iterations; ++i) {
		pthread_mutex_lock(&reactor->invalidation_lock);
		set_clear(&reactor->invalidation_set);
		pthread_mutex_unlock(&reactor->invalidation_lock);

		do (ret = epoll_wait(reactor->epoll_fd, events, __max_events, -1));
		while (ret < 0 && errno == EINTR);
		if (ret < 0) {
			LOG_ERROR("error in epoll_wait: %m");
			reactor->is_running = false;
			return REACTOR_STATUS_ERROR;
		}

		for (j = 0; j < ret; ++j) {
			/*
			 * The event file descriptor is the only one that registers with
			 * a NULL data pointer. We use the NULL to identify it and break
			 * out of the reactor loop.
			 */
			if (events[j].data.ptr == NULL) {
				eventfd_t value;
				eventfd_read(reactor->event_fd, &value);
				reactor->is_running = false;
				return REACTOR_STATUS_STOP;
			}
			object = (reactor_object_t *)events[j].data.ptr;

			pthread_mutex_lock(&reactor->invalidation_lock);
			if (set_contains(&reactor->invalidation_set, object)) {
				pthread_mutex_unlock(&reactor->invalidation_lock);
				continue;
			}

			/* Downgrade the list lock to an object lock. */
			pthread_mutex_lock(&object->lock);
			pthread_mutex_unlock(&reactor->invalidation_lock);
			reactor->object_removed = false;
			if ((events[j].events &
				(EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR))
				&& object->read_ready)
				object->read_ready(object->context);
			if (!reactor->object_removed && (events[j].events & EPOLLOUT)
				&& object->write_ready)
				object->write_ready(object->context);
			pthread_mutex_unlock(&object->lock);

			if (reactor->object_removed) {
				pthread_mutex_destroy(&object->lock);
				free(object);
			}
		}
	}
	reactor->is_running = false;
	return REACTOR_STATUS_DONE;
}

#endif /* OSI_THREADING */
