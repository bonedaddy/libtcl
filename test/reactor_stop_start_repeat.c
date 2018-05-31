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

#include "osi/reactor.h"

int main(void)
{
	int i;
	reactor_t reactor;

	ASSERT(reactor_init(&reactor) == 0);
	for (i = 0; i < 10; ++i) {
		reactor_stop(&reactor);
		reactor_start(&reactor);
	}
	reactor_destroy(&reactor);
	return 0;
}
