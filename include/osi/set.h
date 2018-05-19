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

/*!@file osi/set.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_SET_H
# define __OSI_SET_H

#include "osi/conf.h"

/*!@public
 *
 * @brief
 * The set structure type declaration.
 */
typedef struct set set_t;

/*!@public
 *
 * @brief
 * TODO
 */
typedef size_t (hash_fn_t)(const void *key);

/*!@public
 *
 * @brief
 * TODO
 */
typedef bool (hash_eq_t)(const void *x, const void *y);

/*!@public
 *
 * @brief
 * The set structure definition.
 */
struct set {

	/*! TODO. */
	size_t capacity;

	/*! TODO. */
	size_t length;

	/*! TODO. */
	size_t occupied;

	/*! TODO. */
	size_t upbound;

	/*! TODO. */
	uint8_t *flags;

	/*! TODO. */
	const void **keys;

	/*! TODO. */
	hash_fn_t *hash;

	/*! TODO. */
	hash_eq_t *eq;
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param hash
 * @param eq
 */
__api__ void set_init(set_t *set, hash_fn_t *hash, hash_eq_t *eq);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param idtor
 */
__api__ void set_destroy(set_t *set);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param idtor
 */
__api__ void set_clear(set_t *set);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @return
 */
__api__ size_t set_length(const set_t *set);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param key
 * @return
 */
__api__ bool set_contains(const set_t *set, const void *key);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param key
 * @param value
 * @return
 */
__api__ int set_put(set_t *set, const void *key);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param set
 * @param x
 */
__api__ bool set_remove(set_t *set, const void *key);

#endif /* __OSI_SET_H */
/*!@} */
