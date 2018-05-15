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

#ifdef OS_PROVENCORE
#include <unistd.h>
#endif
#include <osi/thread.h>

typedef struct {
	sema_t *sema;
	int counter;
} dummy_t;

static void *sleep_then_increment_counter(void *context) {
	dummy_t *helper;

	helper = (dummy_t *)context;
	ASSERT(helper);
	ASSERT(helper->sema);
#ifdef OS_PROVENCORE
	sleep(50 * 1000);
#else
	usleep(50 * 1000);
#endif
	++helper->counter;
	sema_post(helper->sema);
	return NULL;
}

int main(void)
{
	sema_t sema;
	thread_t thread;
	dummy_t dummy;

	ASSERT_EQ(0, sema_init(&sema, 0));
	ASSERT_EQ(0, thread_init(&thread, "SEMA_TEST"));

	//ASSERT_FALSE(sema_trywait(&sema));

	dummy.sema = &sema;
	dummy.counter = 0;

	thread_post(&thread, sleep_then_increment_counter, &dummy);
	sema_wait(&sema);
	ASSERT_EQ(1, dummy.counter);

	thread_post(&thread, sleep_then_increment_counter, &dummy);
	sema_wait(&sema);
	ASSERT_EQ(2, dummy.counter);

	thread_post(&thread, sleep_then_increment_counter, &dummy);
	sema_wait(&sema);
	ASSERT_EQ(3, dummy.counter);

	thread_post(&thread, sleep_then_increment_counter, &dummy);
	thread_post(&thread, sleep_then_increment_counter, &dummy);
	sema_wait(&sema);
	sema_wait(&sema);
	ASSERT_EQ(5, dummy.counter);

	sema_destroy(&sema);
	thread_destroy(&thread);
	return 0;
}
