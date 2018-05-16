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
static volatile bool thread_running;

static void *reactor_thread(void *ptr)
{
	reactor_t *reactor;

	reactor = (reactor_t *)ptr;
	thread_running = true;
	reactor_start(reactor);
	thread_running = false;
	return NULL;
}

static void spawn_reactor_thread(reactor_t *reactor)
{
	int ret;

	ret = pthread_create(&thread, NULL, reactor_thread, reactor);
	(void)ret;
	ASSERT(ret == 0);
}

static void join_reactor_thread()
{
	pthread_join(thread, NULL);
}

int main(void)
{
	reactor_t reactor;

	ASSERT_EQ(0, reactor_init(&reactor));
	spawn_reactor_thread(&reactor);
	usleep(50 * 1000);
	ASSERT_TRUE(thread_running);

	reactor_stop(&reactor);
	join_reactor_thread();
	ASSERT_FALSE(thread_running);

	reactor_destroy(&reactor);
	return 0;
}
