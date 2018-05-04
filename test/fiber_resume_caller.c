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

static fiber_t a;
static fiber_t b;

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
	fiber_call(&b, NULL);
	printf("end fiber a\n");
	return NULL;
}

int main(void)
{
	fiber_init(&a, call_a, 32, FIBER_NONE);
	fiber_init(&b, call_b, 32, FIBER_NONE);
	printf("begin main\n");
	fiber_call(&a, NULL);
	printf("end main\n");
	fiber_destroy(&a);
	fiber_destroy(&b);
	return 0;
}
