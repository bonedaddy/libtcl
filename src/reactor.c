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

#ifndef EFD_SEMAPHORE
# define EFD_SEMAPHORE (1 << 0)
#endif /* !EFD_SEMAPHORE */

static reactor_st_t __run_reactor(reactor_t *reactor, int iterations);
static const size_t __max_events = 64;

int reactor_init(reactor_t *reactor)
{
	pollev_t event;

	if (poll_init(&reactor->poll, __max_events)) {
		LOG_ERROR("unable to create epoll instance: %m");
		return -1;
	}
	if (event_init(&reactor->stopev, 0, 0)) {
		LOG_ERROR("unable to create eventfd: %m");
		return -1;
	}
	bzero(&event, sizeof(event));
	event.events = POLL_IN;
	event.ptr = NULL;
	if (poll_add(&reactor->poll, &reactor->stopev, event)) {
		LOG_ERROR("unable to register eventfd with epoll set: %m");
		return -1;
	}
	mutex_init(&reactor->invalidation_lock);
	set_init(&reactor->invalidation_set, NULL, NULL);
	return 0;
}

void reactor_destroy(reactor_t *reactor)
{
	set_destroy(&reactor->invalidation_set);
	poll_destroy(&reactor->poll);
	event_destroy(&reactor->stopev);
	mutex_destroy(&reactor->invalidation_lock);
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
	event_write(&reactor->stopev, 1UL);
}

reactor_object_t *reactor_register(reactor_t *reactor, event_t *ev,
	void *context, reactor_ready_t *read_ready, reactor_ready_t *write_ready)
{
	reactor_object_t *object;
	pollev_t event;

	object = (reactor_object_t *)malloc(sizeof(reactor_object_t));
	object->reactor = reactor;
	object->ev = ev;
	object->context = context;
	object->read_ready = read_ready;
	object->write_ready = write_ready;
	bzero(&event, sizeof(event));
	if (read_ready) event.events |= POLL_IN;
	if (write_ready) event.events |= POLL_OUT;
	event.ptr = object;
	if (poll_add(&reactor->poll, ev, event)) {
		LOG_ERROR("unable to register ev to epoll set: %m");
		free(object);
		return NULL;
	}
	mutex_init(&object->lock);
	return object;
}

void reactor_unregister(reactor_object_t *obj)
{
	reactor_t *reactor = obj->reactor;

	poll_del(&reactor->poll, obj->ev);
	if (reactor->is_running) {
		reactor->object_removed = true;
		return;
	}
	mutex_lock(&reactor->invalidation_lock);
	set_put(&reactor->invalidation_set, obj);
	mutex_unlock(&reactor->invalidation_lock);
	mutex_destroy(&obj->lock);
	free(obj);
}

static reactor_st_t __run_reactor(reactor_t *reactor, int iterations)
{
	int ret, i, j;
	reactor_object_t *object;
	pollev_t events[__max_events];
	
	reactor->is_running = true;
	for (i = 0; iterations == 0 || i < iterations; ++i) {
		mutex_lock(&reactor->invalidation_lock);
		set_clear(&reactor->invalidation_set);
		mutex_unlock(&reactor->invalidation_lock);

		ret = poll_wait(&reactor->poll, events, __max_events);
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
			if (events[j].ptr == NULL) {
				event_value_t value;
				
				event_read(&reactor->stopev, &value);
				reactor->is_running = false;
				return REACTOR_STATUS_STOP;
			}
			object = (reactor_object_t *)events[j].ptr;

			mutex_lock(&reactor->invalidation_lock);
			if (set_contains(&reactor->invalidation_set, object)) {
				mutex_unlock(&reactor->invalidation_lock);
				continue;
			}

			/* Downgrade the list lock to an object lock. */
			mutex_lock(&object->lock);
			mutex_unlock(&reactor->invalidation_lock);
			reactor->object_removed = false;
			if ((events[j].events & POLL_IN) && object->read_ready)
				object->read_ready(object->context);
			if (!reactor->object_removed && (events[j].events & POLL_OUT)
				&& object->write_ready)
				object->write_ready(object->context);
			mutex_unlock(&object->lock);

			if (reactor->object_removed) {
				mutex_destroy(&object->lock);
				free(object);
			}
		}
	}
	reactor->is_running = false;
	return REACTOR_STATUS_DONE;
}
