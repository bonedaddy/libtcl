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

#ifndef CORO_POOL_MMAP
# error "CORO_POOL_MMAP must be defined to compile this file."
#endif

#include <sys/mman.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define BLOCK_TAG_1 0xaabb7788
#define BLOCK_TAG_2 0xccdd9900

struct block_info
{
	/* TAGs for error detection */
	volatile unsigned long tag1;
	struct block_info * volatile next;
	volatile unsigned long tag2;
};

struct page_info
{
	volatile unsigned int use_flag : 1;
	volatile unsigned long head_page : 31;
};

static int g_init_flag;
static size_t g_block_pages;
static size_t g_pool_blocks;
static size_t g_pool_pages;
static void *g_pool_addr;
static struct page_info *g_page_map;
static struct block_info * volatile g_free_list;

#define IN_POOL(addr) ((void *)(addr) < (g_pool_addr + (g_pool_pages<<PAGE_SHIFT)) \
						&& (void *)(addr) >= g_pool_addr)
#define PAGE_NO(addr) ((size_t)(((void *)(addr) - g_pool_addr) >> PAGE_SHIFT))
#define NO_PTR(no) ((no) ? (void *)(g_pool_addr + ((size_t)(no) << PAGE_SHIFT)) : NULL)
#define PTR_HEAD(addr) (NO_PTR(g_page_map[PAGE_NO(addr)].head_page))

FORCEINLINE PURE CONST
static size_t __guard(size_t blocks)
{
	size_t i;

	for (i = 1; blocks &~ ((1UL << i) - 1); ++i);

	return (i + 1);
}

static int do_init_pool()
{
	size_t blk, guard = 0;
	struct block_info *cur = g_pool_addr, *prev = NULL;
	size_t i, blk_page_no;

	for (blk = 0; blk < g_pool_blocks; blk++) {

		/* insert guard page */
		if (blk == guard) {
#ifdef HAS_MPROTECT
			if (mprotect(cur, PAGE_SIZE, PROT_NONE) < 0)
				return -1;
#endif
			/* page_no 0 means NULL */
			g_page_map[PAGE_NO(cur)].head_page = 0;

			cur = (void *)cur + PAGE_SIZE;
			guard = (guard << 1) + 1;
		}

		/* link empty block */
		cur->tag1 = BLOCK_TAG_1;
		cur->tag2 = BLOCK_TAG_2;
		cur->next = NULL;
		if (prev)
			prev->next = cur;
		else
			g_free_list = cur;
		prev = cur;

		blk_page_no = PAGE_NO(cur);
		for (i = 0; i < g_block_pages; i++) {
			g_page_map[blk_page_no + i].head_page = blk_page_no;
		}

		cur = (void *)cur + (g_block_pages << PAGE_SHIFT);
	}
	/* end by a guard-page */
#ifdef HAS_MPROTECT
	if (mprotect(cur, PAGE_SIZE, PROT_NONE) < 0)
		return -1;
#endif
	g_page_map[PAGE_NO(cur)].head_page = 0;
	cur = (void *)cur + PAGE_SIZE;
	/* last check */
	assert(cur == g_pool_addr + (g_pool_pages << PAGE_SHIFT));

	return 0;
}

static int init(void)
{
	if (g_init_flag)
		return 0;

	g_init_flag = 1;
	g_block_pages = (CORO_STACK_SIZE + PAGE_SIZE - 1) >> PAGE_SHIFT;
	g_pool_blocks = CORO_POOL_SIZE;
	if (g_block_pages <= 0 || g_pool_blocks <= 0)
		return -1;
	g_pool_pages = g_block_pages * g_pool_blocks + __guard(g_pool_blocks);

#ifndef MAP_ANONYMOUS
#	define MAP_ANONYMOUS MAP_ANON
#endif
	g_pool_addr = mmap(NULL, (g_pool_pages << PAGE_SHIFT), PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (g_pool_addr == MAP_FAILED) {
		return -2;
	}

	g_page_map = calloc(g_pool_pages, sizeof(struct page_info));
	if (g_page_map == NULL) {
		munmap(g_pool_addr, (g_pool_pages << PAGE_SHIFT));
		return -3;
	}

	if (do_init_pool() < 0) {
		free(g_page_map);
		munmap(g_pool_addr, (g_pool_pages << PAGE_SHIFT));
		return -4;
	}

	return 0;
}

coro_t __corolocate(void *ptr)
{
	if (!IN_POOL(ptr))
		return NULL;
	return PTR_HEAD(ptr);
}

coro_t __coroalloc(void)
{
	struct block_info *block;

	if (init() || !(block = g_free_list))
		return NULL;

	g_free_list = block->next;
	g_page_map[PAGE_NO(block)].use_flag = 1;

	return (coro_t)block;
}

FORCEINLINE
static void __swap32(uint32_t *target, uint32_t *value)
{
	uint32_t tmp = *target;
	*target = *value;
	*value = tmp;
}

void __corofree(coro_t coro)
{
	(void)coro;
}
