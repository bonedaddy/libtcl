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

static int __counter = 0;

void *call(void *arg)
{
	(void)arg;
	++__counter;
	assert(__counter == 2);
	osi_fib_yield(NULL);
	++__counter;
	assert(__counter == 4);
	return NULL;
}

int main(void)
{
	osi_fib_t *fiber;

	fiber = osi_fib_new(call, 32);

	assert(!osi_fib_done(fiber));

	++__counter;
	assert(__counter == 1);

	osi_fib_call(fiber, NULL);

	++__counter;
	assert(__counter == 3);

	assert(!osi_fib_done(fiber));

	osi_fib_call(fiber, NULL);

	++__counter;
	assert(__counter == 5);

	assert(osi_fib_done(fiber));

	return 0;
}
