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

static void fiber_func_0(void *ctx)
{
	for (int i = 0; i < 10; ++i) {
		if (++counter > 10)
			counter = 1;
		assert(counter == *(int *)ctx);
		osi_yield();
	}
}

static void fiber_func_1(void *ctx)
{
	if (++counter > 10)
		counter = 1;
	for (int i = 0; i < 10; ++i) {
		assert(counter == *(int *)ctx);
	}
}

int test_0(void)
{
	int names[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (int i = 0; i < 10; i++)
		osi_fib_create(fiber_func_0, names + i, 1024, 1);
	osi_schedule();
	return 0;
}

int test_1(void)
{
	int names[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (int i = 0; i < 10; i++)
		osi_fib_create(fiber_func_1, names + i, 1024, 1);
	osi_schedule();
	return 0;
}

int main(void)
{
	if (test_0()) return 1;
	if (test_1()) return 1;
	return 0;
}
