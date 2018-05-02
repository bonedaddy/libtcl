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

void *call_return_value(void *arg)
{
	(void)arg;
	assert(!strcmp("result", arg));
	return (NULL);
}

int main(void)
{
	fiber_t *fiber;
	char *result;

	fiber = fiber_new(call_return_value, 32);
	(void)(result = fiber_call(fiber, "result"));
	assert(!result);
	fiber_del(fiber);
	return 0;
}
