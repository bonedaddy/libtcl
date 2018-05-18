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

#include "osi/queue.h"

int main(void)
{
	queue_t queue;

	queue_init(&queue, sizeof(int));
	ASSERT_NULL(queue.buffer);
	ASSERT_EQ(sizeof(int), queue.isize);
	ASSERT_EQ(0, queue.capacity);
	ASSERT_EQ(0, queue.length);
	queue_destroy(&queue, NULL);
	ASSERT_NULL(queue.buffer);
	ASSERT_EQ(sizeof(int), queue.isize);
	ASSERT_EQ(0, queue.capacity);
	ASSERT_EQ(0, queue.length);
	return 0;
}