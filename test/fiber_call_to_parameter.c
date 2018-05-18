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

void *call_to_return_parameter(void *arg)
{
	(void)arg;
	ASSERT(!strcmp("result", arg));
	return (NULL);
}

int main(void)
{
	fid_t fiber;
	char *result;

	fiber_init(&fiber, call_to_return_parameter, 32, FIBER_FL_NONE);
	(void)(result = fiber_call(fiber, "result"));
	ASSERT(!result);
	fiber_destroy(fiber);
	fiber_cleanup();
	return 0;
}
