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

#include "osi/fiber.h"

static char *result[] = { "a", "b", "c", "d" };

void *yield_return_value(void *arg)
{
	printf("%s\n", (char *) arg);
	printf("%s\n", (char *) fiber_yield(result[1]));
	return result[3];
}

int main(void)
{
	fid_t fiber;

	fiber_init(&fiber, yield_return_value, (fiber_attr_t){ });
	printf("%s\n", (char *) fiber_call(fiber, result[0]));
	printf("%s\n", (char *) fiber_call(fiber, result[2]));
	fiber_destroy(fiber);
	fiber_cleanup();
	return 0;
}
