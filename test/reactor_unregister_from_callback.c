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

#include "test.h"

#include "osi/reactor.h"

static pthread_t thread;

static void *reactor_thread(void *ptr)
{
	reactor_t *reactor;

	reactor = (reactor_t *)ptr;
	reactor_start(reactor);
	return NULL;
}

static void spawn_reactor_thread(reactor_t *reactor)
{
	int ret;

	ret = pthread_create(&thread, NULL, reactor_thread, reactor);
	(void)ret;
	ASSERT_EQ(0, ret);
}

static void join_reactor_thread()
{
	pthread_join(thread, NULL);
}

typedef struct {
	reactor_t *reactor;
	reactor_object_t *object;
} unregister_arg_t;

static void unregister_cb(void *context)
{
	unregister_arg_t *arg;

	arg = (unregister_arg_t *)context;
	reactor_unregister(arg->object);
	reactor_stop(arg->reactor);
}

int main(void)
{
	int fd;
	reactor_t reactor;
	unregister_arg_t arg;

	ASSERT_EQ(0, reactor_init(&reactor));
	fd = eventfd(0, 0);
	arg.reactor = &reactor;
	arg.object = reactor_register(&reactor, fd, &arg, unregister_cb, NULL);
	spawn_reactor_thread(&reactor);
	eventfd_write(fd, 1);
	join_reactor_thread();
	close(fd);
	reactor_destroy(&reactor);
	return 0;
}
