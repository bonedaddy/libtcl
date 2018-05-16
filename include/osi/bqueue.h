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

/*!@file osi/bqueue.h
 * @author uael
 *
 * @addtogroup osi.bqueue @{
 */
#ifndef __OSI_BQUEUE_H
# define __OSI_BQUEUE_H

#include <osi/sched.h>
#include <osi/reactor.h>
#include <osi/sema.h>

struct thread;

/*!@public
 *
 * @brief
 * The blocking queue structure declaration.
 */
typedef struct bqueue bqueue_t;

/*!@public
 *
 * @brief
 * The blocking queue listener function callback.
 */
typedef void (listener_t)(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * The blocking queue structure definition.
 */
struct bqueue {

	list_t list;

	uint32_t capacity;

	listener_t *listener;

	sema_t enqueue_sem;

	sema_t dequeue_sem;

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
 * @param bqueue
 * @param capacity
 * @return
 */
__api__ int bqueue_init(bqueue_t *bqueue, unsigned capacity);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @param dtor
 */
__api__ void bqueue_destroy(bqueue_t *bqueue, head_dtor_t *dtor);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @return
 */
__api__ bool bqueue_empty(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @return
 */
__api__ unsigned bqueue_length(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @return
 */
__api__ unsigned bqueue_capacity(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @param ev
 */
__api__ void bqueue_push(bqueue_t *bqueue, head_t *ev);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @return
 */
__api__ head_t *bqueue_pop(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @param ev
 * @return
 */
__api__ bool bqueue_trypush(bqueue_t *bqueue, head_t *ev);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @return
 */
__api__ head_t *bqueue_trypop(bqueue_t *bqueue);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 * @param thread
 * @param listener
 */
__api__ void bqueue_listen(bqueue_t *bqueue, struct thread *thread,
	listener_t *listener);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param bqueue
 */
__api__ void bqueue_unlisten(bqueue_t *bqueue);

#endif /* __OSI_BQUEUE_H */
/*!@} */
