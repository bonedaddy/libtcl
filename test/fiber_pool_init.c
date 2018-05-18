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

#include "osi/fiber/pool.h"

#define N_FIBER 200

int main(void)
{
	fiber_pool_t	pool;
	fiber_t			*fibers[N_FIBER];
	int				i;

	DG();
	fiber_pool_init(&pool);
	DG();
	i = -1;
	while (++i < N_FIBER / 2)
		fibers[i] = fiber_pool_new(&pool);
	DG();

	i = -1;
	while (++i < N_FIBER / 2)
		fiber_pool_ready(&pool, fibers[i]);
	DG();

	i = -1;
	while (++i < N_FIBER / 2)
		fiber_pool_dead(&pool, fibers[i]);
	DG();

	i = -1;
	while (++i < N_FIBER / 2) {
		DG();
		fiber_pool_ready(&pool, fibers[i]);
	}
	DG();

	i = N_FIBER / 2 - 1;
	while (++i < N_FIBER)
		fibers[i] = fiber_pool_new(&pool);
	DG();

	i = N_FIBER / 2 - 1;
	while (++i < N_FIBER)
		fiber_pool_ready(&pool, fibers[i]);
	DG();

	i = N_FIBER / 2 - 1;
	while (++i < N_FIBER)
		fiber_pool_dead(&pool, fibers[i]);
	DG();

	fiber_pool_destroy(&pool);
	DG();
	return 0;
}
