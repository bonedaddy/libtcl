/*
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

#include "coro/internal.h"

#ifndef CORO_POOL_MALLOC
# error "CORO_POOL_MALLOC must be defined to compile this file."
#endif

static size_t g_alloc_size = CORO_STACK_SIZE;
static size_t g_pool_size = CORO_POOL_SIZE;

static bool __isinit;
static void **g_alloc_table;
static void *g_max_addr = NULL;

static int __init(void)
{
	if (__isinit)
		return 0;
	__isinit = 1;
	if (!(g_alloc_table = calloc(g_pool_size, sizeof(void*)))) {
		return -1;
	}
	return 0;
}

coro_t __corolocate(void *ptr)
{

	size_t i;
	__init();
	if (ptr > g_max_addr)
		return NULL;
	for (i = 0; i < g_pool_size; i++) {
		if (ptr >= g_alloc_table[i] && ptr < g_alloc_table[i] + g_alloc_size) {
			return g_alloc_table[i];
		}
	}
	return NULL;
}

coro_t __coroalloc(void)
{
	static size_t index = 0;
	void *ptr = NULL;
	size_t i;
	__init();
	for (i = 0; i < g_pool_size && !ptr; i++) {
		if (g_alloc_table[index] == NULL) {
			ptr = g_alloc_table[index] = malloc(g_alloc_size);
		}
		index = (index + 1) % g_pool_size;
	}
	if (ptr + g_alloc_size - 1 > g_max_addr)
		g_max_addr = ptr + g_alloc_size - 1;
	return ptr;
}

void __corofree(coro_t coro)
{
	size_t i;
	for (i = 0; i < g_pool_size; i++) {
		if (g_alloc_table[i] == coro) {
			g_alloc_table[i] = NULL;
			free(coro);
			return;
		}
	}
}
