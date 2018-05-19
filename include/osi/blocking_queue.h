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

/*!@file osi/blocking_queue.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_BLOCKING_QUEUE_H
# define __OSI_BLOCKING_QUEUE_H

#include "osi/reactor.h"
#include "osi/sema.h"

struct thread;

/*!@public
 *
 * @brief
 * The blocking queue structure declaration.
 */
typedef struct blocking_queue blocking_queue_t;

/*!@public
 *
 * @brief
 * The blocking queue listener function callback.
 */
typedef void (listener_t)(blocking_queue_t *blocking_queue);

/*!@public
 *
 * @brief
 * The blocking queue structure definition.
 */
struct blocking_queue {

	list_t list;

	uint32_t capacity;

	listener_t *listener;

	sema_t producer;

	sema_t consumer;

#ifdef OSI_THREADING

	pthread_mutex_t lock;

	reactor_object_t *reactor_object;
#else

	struct thread *thread;
#endif /* OSI_THREADING */
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @param capacity
 * @return
 */
__api__ int blocking_queue_init(blocking_queue_t *queue, unsigned capacity);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @param dtor
 */
__api__ void blocking_queue_destroy(blocking_queue_t *queue, head_dtor_t *dtor);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @return
 */
__api__ bool blocking_queue_empty(blocking_queue_t *queue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @return
 */
__api__ unsigned blocking_queue_length(blocking_queue_t *queue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @return
 */
__api__ unsigned blocking_queue_capacity(blocking_queue_t *queue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @param ev
 */
__api__ void blocking_queue_push(blocking_queue_t *queue, head_t *ev);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @return
 */
__api__ head_t *blocking_queue_pop(blocking_queue_t *queue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @param node
 * @return
 */
__api__ bool blocking_queue_trypush(blocking_queue_t *queue, head_t *node);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @return
 */
__api__ head_t *blocking_queue_trypop(blocking_queue_t *queue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 * @param thread
 * @param listener
 */
__api__ void blocking_queue_listen(blocking_queue_t *queue,
	struct thread *thread, listener_t *listener);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param queue
 */
__api__ void blocking_queue_unlisten(blocking_queue_t *queue);

#endif /* __OSI_BLOCKING_QUEUE_H */
/*!@} */
