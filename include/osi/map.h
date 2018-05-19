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

/*!@file osi/map.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_MAP_H
# define __OSI_MAP_H

#include "osi/conf.h"

/*!@public
 *
 * @brief
 * The map structure type declaration.
 */
typedef struct map map_t;

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
 * map item destruction callback, passed to `map_destroy' to destroy every
 * item before freed.
 */
typedef void (map_dtor_t)(void *item);

/*!@public
 *
 * @brief
 * The map structure definition.
 */
struct map {

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
	const void **values;

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
 * @param map
 * @param hash
 * @param eq
 */
__api__ void map_init(map_t *map, hash_fn_t *hash, hash_eq_t *eq);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param idtor
 */
__api__ void map_destroy(map_t *map, map_dtor_t *idtor);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param idtor
 */
__api__ void map_clear(map_t *map, map_dtor_t *idtor);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @return
 */
__api__ size_t map_length(const map_t *map);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param key
 * @return
 */
__api__ bool map_contains(const map_t *map, const void *key);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param key
 * @return
 */
__api__ void *map_get(const map_t *map, const void *key);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param key
 * @param value
 * @return
 */
__api__ int map_set(map_t *map, const void *key, const void *value);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param map
 * @param x
 */
__api__ bool map_remove(map_t *map, const void *key);

#endif /* __OSI_MAP_H */
/*!@} */
