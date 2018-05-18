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

/*!@file osi/queue.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_QUEUE_H
# define __OSI_QUEUE_H

#include "osi/vector.h"

/*!@public
 *
 * @brief
 * The queue structure type declaration.
 */
typedef union queue queue_t;

/*!@public
 *
 * @brief
 * QUEUE item destruction callback, passed to `queue_destroy' to destroy every
 * item before freed.
 */
typedef void (queue_dtor_t)(void *item);

/*!@public
 *
 * @brief
 * The queue structure definition.
 */
union queue {

	/*!@private
	 * Base of the queue. */
	vector_t base;

	/*! Public accessors. */
	struct {

		/*! Dynamic buffer to store items. */
		void *buffer;

		/*! Size of an item which is passed to `queue_init'. */
		size_t isize;

		/*! The queue capacity, or size of the allocated `buffer'. */
		size_t capacity;

		/*! The queue length, represent the number of items stored in
		 * `buffer'.
		 */
		size_t length;
	};
};

__api__ void queue_init(queue_t *queue, size_t isize);

__api__ void queue_destroy(queue_t *queue, queue_dtor_t *idtor);

__api__ void queue_clear(queue_t *queue, queue_dtor_t *idtor);

__api__ size_t queue_length(queue_t *queue);

__api__ void *queue_peek(queue_t *queue);

__api__ void queue_ensure(queue_t *queue, size_t n);

__api__ void queue_grow(queue_t *queue, size_t n_added);

__api__ void *queue_npush(queue_t *queue, size_t n);

__api__ size_t queue_npop(queue_t *queue, size_t n, void *out);

__api__ void *queue_push(queue_t *queue);

__api__ bool queue_pop(queue_t *queue, void *out);

#endif /* __OSI_QUEUE_H */
/*!@} */
