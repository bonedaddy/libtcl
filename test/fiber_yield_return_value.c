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

static char *result[] = { "a\n", "b\n", "c\n", "d\n" };

void *yield_return_value(void *arg)
{
	printf(arg);
	printf(fiber_yield(result[1]));
	return result[3];
}

int main(void)
{
	fiber_t *fiber;

	fiber = fiber_new(yield_return_value, 32);

	printf(fiber_call(fiber, result[0]));
	printf(fiber_call(fiber, result[2]));

	fiber_del(fiber);
	return 0;
}
