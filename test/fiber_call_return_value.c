/*
 * Copyright (C) 2014 Google, Inc.
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

#include "osi/fiber.h"

void *call_return_value(void *arg)
{
	static char result[] = "result";

	(void)arg;
	return result;
}

int main(void)
{
	fid_t fiber;
	char *result;

	fiber_init(&fiber, call_return_value, (fiber_attr_t){ });
	(void)(result = fiber_call(fiber, NULL));
	ASSERT(!strcmp("result", result));
	fiber_destroy(fiber);
	fiber_cleanup();
	return 0;
}
