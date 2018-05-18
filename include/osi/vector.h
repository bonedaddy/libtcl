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

/*!@file osi/vector.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_VECTOR_H
# define __OSI_VECTOR_H

#include "osi/conf.h"

/*!@public
 *
 * @brief
 * The vector structure type declaration.
 */
typedef struct vector vector_t;

/*!@public
 *
 * @brief
 * Vector item destruction callback, passed to `vector_destroy' to destroy every
 * item before freed.
 */
typedef void (vector_dtor)(void *item);

/*!@public
 *
 * @brief
 * The vector structure definition.
 */
struct vector {

	/*! Dynamic buffer to store items. */
	void *buffer;

	/*! Size of an item which is passed to `vector_init'. */
	size_t isize;

	/*! The vector capacity, or size of the allocated `buffer'. */
	size_t capacity;

	/*! The vector length, represent the number of items stored in `buffer'. */
	size_t length;
};

/*!
 *
 * @param vector
 * @param isize
 */
__api__ void vector_init(vector_t *vector, size_t isize);

/*!
 *
 * @param vector
 * @param idtor
 */
__api__ void vector_destroy(vector_t *vector, vector_dtor *idtor);

/*!
 *
 * @param vector
 * @param idtor
 */
__api__ void vector_clear(vector_t *vector, vector_dtor *idtor);

/*!
 *
 * @param vector
 * @return
 */
__api__ size_t vector_length(vector_t *vector);

/*!
 *
 * @param vector
 * @return
 */
__api__ void *vector_begin(vector_t *vector);

__api__ void *vector_end(vector_t *vector);

__api__ void *vector_back(vector_t *vector);

__api__ void *vector_at(vector_t *vector, size_t i);

__api__ void vector_ensure(vector_t *vector, size_t n);

__api__ void vector_grow(vector_t *vector, size_t n_added);

__api__ void *vector_npush_back(vector_t *vector, size_t n);

__api__ void *vector_npush_front(vector_t *vector, size_t n);

__api__ void *vector_npush_at(vector_t *vector, size_t n, size_t i);

__api__ size_t vector_npop_back(vector_t *vector, size_t n, void *out);

__api__ size_t vector_npop_front(vector_t *vector, size_t n, void *out);

__api__ size_t vector_npop_at(vector_t *vector, size_t n, size_t i, void *out);

__api__ void *vector_push_back(vector_t *vector);

__api__ void *vector_push_front(vector_t *vector);

__api__ void *vector_push_at(vector_t *vector, size_t i);

__api__ bool vector_pop_back(vector_t *vector, void *out);

__api__ bool vector_pop_front(vector_t *vector, void *out);

__api__ bool vector_pop_at(vector_t *vector, size_t i, void *out);

#define vector_foreach(vector, item) \
	for (item = vector_begin(vector); \
		item && (void *)item != vector_end(vector); \
		item = (void *)((char *)item + (vector)->isize))

#define vector_rforeach(vector, item) \
	for (item = vector_back(vector); \
		item && (void *)item >= vector_begin(vector); \
		item = (void *)((char *)item - (vector)->isize))

#endif /* __OSI_VECTOR_H */
/*!@} */
