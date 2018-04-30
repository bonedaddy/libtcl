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
#include <stdio.h>
#include <string.h>
#include <strings.h>

static void fiber_func(void *ctx)
{
	for (int i = 0; i < 10; ++i) {
		printf("%d [%d/%d]\n", *(int *)ctx, i + 1, 10);
		osi_yield();
	}
	printf("%d done\n", *(int *)ctx);
}

int main(void) {
	int names[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (int i = 0; i < 10; i++)
		osi_fib_create(fiber_func, names + i, 1024, 1);
	osi_schedule();
	return 0;
}
