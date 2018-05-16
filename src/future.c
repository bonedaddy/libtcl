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

#include <osi/future.h>

static void __future_free(future_t *future)
{
	sema_destroy(&future->sema);
}

int future_init(future_t *future)
{
	int st;

	if ((st = sema_init(&future->sema, 0)))
		return st;
	future->awaitable = true;
	future->result = NULL;
	return 0;
}

void future_immediate(future_t *future, void const *value)
{
	future->awaitable = false;
	future->result = value;
}

void future_ready(future_t *future, void *value)
{
	future->awaitable = false;
	future->result = value;
	sema_post(&future->sema);
}

void *future_await(future_t *future)
{
	if (future->awaitable)
		sema_wait(&future->sema);
	__future_free(future);
	return (void *)future->result;
}