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
#include <stdio.h>

static int counter = 0;

static void *fiber_func_0(void *ctx)
{
	(void)ctx;
	for (int i = 0; i < 10; ++i) {
		if (++counter > 10)
			counter = 1;
		printf("%d [%d/%d]\n", *(int *)ctx, i, 10);
		assert(counter == *(int *)ctx);
		fiber_yield(NULL);
	}
	return NULL;
}

static void *fiber_func_1(void *ctx)
{
	(void)ctx;
	if (++counter > 10)
		counter = 1;
	for (int i = 0; i < 10; ++i) {
		printf("%d [%d/%d]\n", *(int *)ctx, i, 10);
		assert(counter == *(int *)ctx);
	}
	return NULL;
}

static void test_0(void)
{
	int names[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	sched_t sched;

	counter = 0;
	sched_init(&sched);
	for (int i = 0; i < 10; i++) {
		sched_spawn(&sched, fiber_new(fiber_func_0, 32), names + i, 1);
	}
	assert(counter == 0);
	sched_start(&sched);
	assert(counter == 10);
}

static void test_1(void)
{
	int names[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	sched_t sched;

	counter = 0;
	sched_init(&sched);
	for (int i = 0; i < 10; i++) {
		sched_spawn(&sched, fiber_new(fiber_func_1, 32), names + i, 1);
	}
	assert(counter == 0);
	sched_start(&sched);
	assert(counter == 10);
}

int main(void)
{
	test_0();
	test_1();
	return 0;
}
