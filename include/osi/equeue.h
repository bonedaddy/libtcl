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

/*!@file osi/equeue.h
 * @author uael
 *
 * @addtogroup osi.equeue @{
 */
#ifndef __OSI_EQUEUE_H
# define __OSI_EQUEUE_H

#include <osi/sched.h>
#include <osi/reactor.h>
#include <osi/sema.h>

struct thread;

/*!@public
 *
 * @brief
 * The event queue structure declaration.
 */
typedef struct equeue equeue_t;

/*!@public
 *
 * @brief
 * The event queue listener function callback.
 */
typedef void (listener_t)(equeue_t *equeue);

/*!@public
 *
 * @brief
 * The event queue structure definition.
 */
struct equeue {

	list_t list;

	uint32_t capacity;

	listener_t *listener;

	sema_t enqueue_sem;

	sema_t dequeue_sem;

#ifdef OSI_THREADING

	pthread_mutex_t lock;

	reactor_object_t *reactor_object;
#endif /* OSI_THREADING */
};

__api__ int equeue_init(equeue_t *equeue, unsigned capacity);

__api__ void equeue_destroy(equeue_t *equeue, node_dtor_t *dtor);

__api__ bool equeue_empty(equeue_t *equeue);

__api__ size_t equeue_length(equeue_t *equeue);

__api__ void equeue_push(equeue_t *equeue, node_t *ev);

__api__ node_t *equeue_pop(equeue_t *equeue);

__api__ bool equeue_trypush(equeue_t *equeue, node_t *ev);

__api__ node_t *equeue_trypop(equeue_t *equeue);

__api__ void equeue_listen(equeue_t *equeue, struct thread *thread,
	listener_t *listener);

__api__ void equeue_unlisten(equeue_t *equeue);

#endif /* __OSI_EQUEUE_H */
/*!@} */
