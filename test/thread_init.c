/*
 * Copyright (C) 2018 Tempow
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

#include "osi/thread.h"

int main(void)
{
	thread_t thread;

	ASSERT_EQ(0, thread_init(&thread, "ZOB"));
	ASSERT_STREQ(thread.name, "ZOB");
	thread_destroy(&thread);

	ASSERT_EQ(0, thread_init(&thread, "ZEB"));
	ASSERT_STREQ(thread.name, "ZEB");
	thread_destroy(&thread);
	return 0;
}
