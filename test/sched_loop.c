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

#include "osi/sched.h"

static int counter = 0;
static sched_t sched;

static void *loop1(void *arg)
{
	(void)arg;
	printf("loop1: %d\n", ++counter);
	return (NULL);
}

static void *loop2(void *arg)
{
	(void)arg;
	printf("loop2: %d\n", ++counter);
	return (NULL);
}

static void *stop(void *arg)
{
	(void)arg;
	if (counter >= 42)
		sched_stop(&sched);
	return (NULL);
}

int main(void)
{
	sched_init(&sched);
	sched_loop(&sched, loop1, 32, NULL, 1);
	sched_loop(&sched, loop2, 32, NULL, 1);
	sched_loop(&sched, stop, 32, NULL, 1);
	sched_start(&sched);
	return 0;
}
