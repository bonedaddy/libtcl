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

#pragma once

/*!@file osi/stack.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_STACK_H
# define __OSI_STACK_H

#include "osi/vector.h"

/*!@public
 *
 * @brief
 * The stack structure type declaration.
 */
typedef union stack stack_t;

/*!@public
 *
 * @brief
 * STACK item destruction callback, passed to `stack_destroy' to destroy every
 * item before freed.
 */
typedef void (stack_dtor)(void *item);

/*!@public
 *
 * @brief
 * The stack structure definition.
 */
union stack {

	/*!@private
	 * Base of the stack. */
	vector_t base;

	/*! Public accessors. */
	struct {

		/*! Dynamic buffer to store items. */
		const void *buffer;

		/*! Size of an item which is passed to `stack_init'. */
		const size_t isize;

		/*! The stack capacity, or size of the allocated `buffer'. */
		const size_t capacity;

		/*! The stack length, represent the number of items stored in
		 * `buffer'.
		 */
		const size_t length;
	};
};

__api__ void stack_init(stack_t *stack, size_t isize);

__api__ void stack_destroy(stack_t *stack, stack_dtor *idtor);

__api__ void stack_clear(stack_t *stack, stack_dtor *idtor);

__api__ size_t stack_length(stack_t *stack);

__api__ void *stack_peek(stack_t *stack);

__api__ void stack_ensure(stack_t *stack, size_t n);

__api__ void stack_grow(stack_t *stack, size_t n_added);

__api__ void *stack_npush(stack_t *stack, size_t n);

__api__ size_t stack_npop(stack_t *stack, size_t n, void *out);

__api__ void *stack_push(stack_t *stack);

__api__ bool stack_pop(stack_t *stack, void *out);

#endif /* __OSI_STACK_H */
/*!@} */
