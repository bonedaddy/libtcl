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

/*!@file osi/stack.h
 * @author uael
 *
 * @addtogroup osi.stack @{
 */
#ifndef __OSI_STACK_H
# define __OSI_STACK_H

#include <pp.h>

#define OSI_STACK_MINCAP 32

#define OSI_STACK_DECLARE(SCOPE, NAME, T, TSize) \
	typedef struct NAME { \
		T *head; \
		TSize len; \
		TSize cap; \
	} NAME##_t; \
	SCOPE void NAME##_ctor(NAME##_t *stack); \
	SCOPE void NAME##_dtor(NAME##_t *stack); \
	SCOPE TSize NAME##_len(NAME##_t *stack); \
	SCOPE T *NAME##_peek(NAME##_t *stack);\
	SCOPE T *NAME##_push(NAME##_t *stack, TSize n); \
	SCOPE TSize NAME##_pop(NAME##_t *stack, TSize n);

#define OSI_STACK_IMPL(SCOPE, NAME, T, TSize) \
	SCOPE void NAME##_ctor(NAME##_t *stack) { \
		bzero(stack, sizeof(NAME##_t)); \
	} \
	SCOPE void NAME##_dtor(NAME##_t *stack) { \
		if (stack->head) { \
			free(stack->head); \
			bzero(stack, sizeof(NAME##_t)); \
		} \
	} \
	SCOPE TSize NAME##_len(NAME##_t *stack) { \
		 return stack->len; \
	} \
	SCOPE T *NAME##_peek(NAME##_t *stack) { \
		if (stack->head) \
		 	return stack->head + (stack->len - 1); \
		return NULL; \
	} \
	SCOPE T *NAME##_push(NAME##_t *stack, TSize n) { \
	 	T *buf; \
	 	TSize cap; \
		if (stack->len + n > stack->cap) { \
		 	cap = stack->cap ? stack->cap : OSI_STACK_MINCAP; \
		 	do cap *= 2; while (stack->len + n > cap); \
			if (!(buf = (T *)realloc(stack->head, cap))) \
				return NULL; \
			stack->head = buf; \
			stack->cap = cap; \
		} \
		buf = stack->head + stack->len; \
		bzero(buf, n * sizeof(T)); \
		stack->len += n; \
		return buf; \
	} \
	SCOPE TSize NAME##_pop(NAME##_t *stack, TSize n) { \
		if (!stack->len) \
			return 0; \
		if (n > stack->len) \
			n = stack->len; \
		stack->len -= n; \
		return n; \
	}

#endif /* __OSI_STACK_H */
/*!@} */
