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

#include <osi/equeue.h>

static const char *DUMMY_DATA_STRING = "Dummy data string";

typedef struct {
	head_t hold;
	char const *data;
} dummy_t;

int main(void)
{
	equeue_t equeue;
	dummy_t dummy, *entry;
	head_t *head;

	ASSERT_EQ(0, equeue_init(&equeue, 1));
	ASSERT_EQ(0, equeue_length(&equeue));
	ASSERT_TRUE(equeue_empty(&equeue));

	head_init(&dummy.hold);
	dummy.data = DUMMY_DATA_STRING;

	ASSERT_TRUE(equeue_trypush(&equeue, &dummy.hold));
	ASSERT_FALSE(equeue_empty(&equeue));

	ASSERT(head = equeue_trypop(&equeue));
	entry = LIST_ENTRY(head, dummy_t, hold);
	ASSERT_STREQ(DUMMY_DATA_STRING, entry->data);

	equeue_destroy(&equeue, NULL);
	return 0;
}
