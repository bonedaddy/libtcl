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

#include <osi/thread.h>

#ifndef OSI_THREADING
static sema_t full;
static sema_t empty;
static int item_slot;
static int counter = 0;

void *consumer(void *ctx)
{
	int item;
	int i;

	(void)ctx;
	for (i = 0; i < 10; ++i) {
		sema_wait(&full);
		item = item_slot;
		printf("got %d !\n", item);
		ASSERT_EQ(counter, item);
		++counter;
		sema_post(&empty);
	}
	return NULL;
}

void *producer(void *ctx)
{
	int item;
	int i;

	(void)ctx;
	for (i = 0; i < 10; ++i) {
		item = i;
		sema_wait(&empty);
		item_slot = item;
		sema_post(&full);
	}
	return NULL;
}
#endif

int main(void)
{
#ifndef OSI_THREADING
	thread_t thread;

	ASSERT_EQ(0, thread_init(&thread, "SEMA_TEST"));
	ASSERT_EQ(0, sema_init(&full, 0));
	ASSERT_EQ(0, sema_init(&empty, 1));

	thread_post(&thread, producer, NULL);
	thread_post(&thread, consumer, NULL);

	thread_destroy(&thread);
	sema_destroy(&full);
	sema_destroy(&empty);
#endif
	return 0;
}
