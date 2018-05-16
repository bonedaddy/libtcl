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

#include <osi/bqueue.h>

int main(void)
{
	bqueue_t bqueue;

	ASSERT_EQ(0, bqueue_init(&bqueue, 0));
	ASSERT_EQ(0, bqueue_length(&bqueue));
	ASSERT_TRUE(bqueue_empty(&bqueue));
	bqueue_destroy(&bqueue, NULL);

	ASSERT_EQ(0, bqueue_init(&bqueue, 1));
	ASSERT_EQ(0, bqueue_length(&bqueue));
	ASSERT_TRUE(bqueue_empty(&bqueue));
	bqueue_destroy(&bqueue, NULL);
	return 0;
}
