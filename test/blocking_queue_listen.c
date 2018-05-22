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

#include "osi/blocking_queue.h"
#include "osi/thread.h"

#define TEST_QUEUE_SIZE 10

static int counter = 0;

static void onready(blocking_queue_t *queue)
{
	const char *item;

	++counter;
	item = (const char *)blocking_queue_pop(queue);
	ASSERT_STREQ("ZOB", item);
}

int main(void)
{
	thread_t thread;
	blocking_queue_t queue;

	thread_init(&thread, "T1");
	blocking_queue_init(&queue, TEST_QUEUE_SIZE);

	blocking_queue_listen(&queue, &thread, onready);

	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");
	blocking_queue_push(&queue, "ZOB");

	blocking_queue_destroy(&queue, NULL);
	thread_destroy(&thread);

	ASSERT_EQ(13, counter);

	fiber_cleanup();
	return 0;
}
