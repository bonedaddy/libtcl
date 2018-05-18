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

static fid_t a;
static fid_t b;
static fid_t c;
static fid_t d;
static fid_t e;

static int counter = 0;

void *call_a(void *arg)
{
	(void)arg;
	ASSERT_EQ(2, counter);
	++counter;
	return NULL;
}

void *call_b(void *arg)
{
	(void)arg;
	ASSERT_EQ(1, counter);
	++counter;
	return NULL;
}

void *call_c(void *arg)
{
	(void)arg;
	ASSERT_EQ(3, counter);
	++counter;
	return NULL;
}

void *call_d(void *arg)
{
	(void)arg;
	ASSERT_EQ(0, counter);
	++counter;
	return NULL;
}

void *call_e(void *arg)
{
	(void)arg;
	ASSERT_EQ(4, counter);
	++counter;
	return NULL;
}

int main(void)
{
	fiber_init(&a, call_a, (fiber_attr_t){ .priority = 1 });
	fiber_init(&b, call_b, (fiber_attr_t){ .priority = 2 });
	fiber_init(&c, call_c, (fiber_attr_t){ .priority = 0 });
	fiber_init(&d, call_d, (fiber_attr_t){ .priority = 4 });
	fiber_init(&e, call_e, (fiber_attr_t){ .priority = -1 });
	fiber_join(e);
	ASSERT_EQ(5, counter);
	fiber_cleanup();
	return 0;
}
