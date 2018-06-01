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

#include "osi/coroutine.h"

static void *hello(void *arg)
{
	(void)arg;
	write(1, "Hello", 5);
	coro_yield(NULL);
	write(1, "Zob", 3);
	coro_yield(NULL);
	write(1, "World !", 7);
	return NULL;
}

int main(void)
{
	coro_t coro;

	coro_init(&coro, hello);
	coro_resume(coro, NULL);
	write(1, " ", 1);
	coro_resume(coro, NULL);
	write(1, " ", 1);
	coro_resume(coro, NULL);
	return 0;
}
