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

/*!@file osi/event.h
 * @author uael
 *
 * @addtogroup osi.event @{
 */
#ifndef __OSI_EVENT_H
# define __OSI_EVENT_H

#include <osi/sched.h>

/*!@public
 *
 * @brief
 * The event structure.
 */
typedef struct event event_t;

/*!@public
 *
 * @brief
 * The event structure implementation.
 */
struct event {
	uint16_t event;
	uint16_t len;
	uint16_t offset;
	uint16_t layer;
	uint8_t data[];
};

typedef struct equeue equeue_t;

typedef void *(listener_t)(equeue_t *ev);

struct equeue {
	event_t **buf;
	size_t slot;
	size_t size;
	listener_t *listener;
	sched_t *sched;
};

__api__ void equeue_init(equeue_t *equeue);

__api__ void equeue_destroy(equeue_t *equeue);

__api__ void equeue_push_silent(equeue_t *equeue, event_t *ev);

__api__ void equeue_push(equeue_t *equeue, event_t *ev);

__api__ event_t *equeue_pop(equeue_t *equeue);

__api__ void equeue_listen(equeue_t *equeue, sched_t *s, listener_t *listener);

#endif /* __OSI_EVENT_H */
/*!@} */
