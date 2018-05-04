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

#include <osi/fiber.h>
#include <osi/sched.h>

#include <assert.h>

static int counter = 0;

void *call(void *arg)
{
	(void)arg;
	++counter;
	assert(counter == 2);
	fiber_yield(NULL);
	++counter;
	assert(counter == 4);
	return NULL;
}

int main(void)
{
	fiber_t fiber;

	fiber_init(&fiber, call, 32, FIBER_NONE);
	assert(!fiber_isdone(&fiber));
	++counter;
	assert(counter == 1);
	fiber_call(&fiber, NULL);
	++counter;
	assert(counter == 3);
	assert(!fiber_isdone(&fiber));
	fiber_call(&fiber, NULL);
	++counter;
	assert(counter == 5);
	assert(fiber_isdone(&fiber));
	fiber_destroy(&fiber);
	return 0;
}
