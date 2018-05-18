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

#define LOG_TAG "test"

#include "test.h"

#include "osi/thread.h"
#include "osi/log.h"

static sema_t sema;
static int counter;

void *before(void * param)
{
	(void)param;
	ASSERT_EQ(0, counter);
	++counter;
	sema_post(&sema);
	return(NULL);
}

void *after(void * param)
{
	(void)param;
	sema_wait(&sema);
	ASSERT_EQ(1, counter);
	++counter;
	return(NULL);
}

int main(void)
{
	thread_t thread;

	ASSERT_EQ(0, sema_init(&sema, 0));
	ASSERT_EQ(0, thread_init(&thread, "SEMA_TEST"));

	counter = 0;
	thread_post(&thread, after, NULL);
	thread_post(&thread, before, NULL);

	thread_destroy(&thread);
	ASSERT_EQ(2, counter);
	sema_destroy(&sema);
	fiber_cleanup();
	return 0;
}
