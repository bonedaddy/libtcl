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

/*!@file osi/list.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_LIST_H
# define __OSI_LIST_H

#include <osi/conf.h>
#include <stdint.h>
#include <stddef.h>

/*!@public
 *
 * @brief
 * TODO
 */
typedef struct node node_t;

/*!@public
 *
 * @brief
 * TODO
 */
typedef struct list list_t;

struct node {

	/** Predecessor */
	node_t *pred;

	/** Successor */
	node_t *succ;
};

struct list {

	/** Predecessor */
	node_t *pred;

	/** Successor */
	node_t *succ;

	/** The count in the list */
	size_t len;
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param node
 */
__api__ void node_init(node_t *node);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 */
__api__ void list_init(list_t *list);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @param entry
 */
__api__ void list_unshift(list_t *list, node_t *entry);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @param entry
 */
__api__ void list_push(list_t *list, node_t *entry);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @param entry
 */
__api__ void list_detach(list_t *list, node_t *entry);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @return
 */
__api__ node_t *list_shift(list_t *list);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @return
 */
__api__ node_t *list_pop(list_t *list);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @param entry
 * @return
 */
__api__ bool list_contains(list_t *list, node_t *entry);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list
 * @return
 */
#define LIST_INITIALIZER(list) \
	{ (node_t *) &(list), (node_t *) &(list), 0 }

/*!@public
 *
 * @brief
 * TODO
 *
 * @param list_ptr
 * @param app_type
 * @param list_member
 * @return
 */
#define LIST_ENTRY(list_ptr, app_type, list_member) \
    ((app_type *) (((char *) (list_ptr)) - offsetof(app_type, list_member)))

#endif /* __OSI_LIST_H */
/*!@} */
