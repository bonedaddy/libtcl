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

#define TEST_QUEUE_SIZE 10

static const char *DUMMY_DATA_STRING = "Dummy data string";

typedef struct {
	head_t hold;
	char const *data;
} dummy_t;

int main(void)
{
	unsigned i;
	bqueue_t bqueue;
	dummy_t dummy, *entry, dummies[TEST_QUEUE_SIZE + 1];
	head_t *head;

	ASSERT_EQ(0, bqueue_init(&bqueue, TEST_QUEUE_SIZE));

	head_init(&dummy.hold);
	dummy.data = DUMMY_DATA_STRING;

	bqueue_push(&bqueue, &dummy.hold);
	ASSERT(head = bqueue_pop(&bqueue));
	entry = LIST_ENTRY(head, dummy_t, hold);
	ASSERT_STREQ(DUMMY_DATA_STRING, entry->data);

	ASSERT_TRUE(bqueue_trypush(&bqueue, &dummy.hold));
	ASSERT(head = bqueue_trypop(&bqueue));
	entry = LIST_ENTRY(head, dummy_t, hold);
	ASSERT_STREQ(DUMMY_DATA_STRING, entry->data);

	for (i = 0; i < TEST_QUEUE_SIZE; ++i) {
		head_init(&dummies[i].hold);
		dummies[i].data = DUMMY_DATA_STRING;
		ASSERT_TRUE(bqueue_trypush(&bqueue, &dummies[i].hold));
	}
	head_init(&dummies[i].hold);
	dummies[i].data = DUMMY_DATA_STRING;
	ASSERT_FALSE(bqueue_trypush(&bqueue, &dummies[i].hold));

	for (i = 0; i < TEST_QUEUE_SIZE; ++i) {
		ASSERT(head = bqueue_trypop(&bqueue));
		entry = LIST_ENTRY(head, dummy_t, hold);
		ASSERT_STREQ(DUMMY_DATA_STRING, entry->data);
	}

	ASSERT_NULL(bqueue_trypop(&bqueue));

	bqueue_destroy(&bqueue, NULL);
	return 0;
}
