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

static osi_fib_t *a;
static osi_fib_t *b;

void *call_b(void *arg)
{
	(void)arg;
	printf("fiber b\n");
	return NULL;
}

void *call_a(void *arg)
{
	(void)arg;
	printf("begin fiber a\n");
	osi_fib_call(b, NULL);
	printf("end fiber a\n");
	return NULL;
}

int main(void)
{
	a = osi_fib_new(call_a, 32);
	b = osi_fib_new(call_b, 32);
	printf("begin main\n");
	osi_fib_call(a, NULL);
	printf("end main\n");
	osi_fib_delete(a);
	osi_fib_delete(b);
	return 0;
}
