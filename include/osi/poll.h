/*
 * Copyright (C) 2018 Tempow
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

/*!@file osi/poll.h
 * @author uael
 *
 * @addtogroup osi.reactor @{
 */
#ifndef __OSI_POLL_H
# define __OSI_POLL_H

#include "osi/event.h"
#include "osi/map.h"

#define POLL_IN_ (1 << 0)
#define POLL_OUT_ (1 << 1)

/*!@public
 *
 * @brief
 * The poll structure declaration.
 */
typedef struct poll poll_t;

/*!@public
 *
 * @brief
 * The poll event structure declaration.
 */
typedef struct pollev pollev_t;

/*!@public
 *
 * @brief
 * The reactor poll structure definition.
 */
struct poll {

#ifdef OSI_THREADING

	/*! TODO */
	int fd;
#else

	/*! TODO */
	map_t events;

	/*! TODO */
	size_t size;
#endif
};

struct pollev {

	/*! TODO */
	uint32_t events;

	/*! TODO */
	void *ptr;
};

__api int poll_init(poll_t *poll, size_t size);

__api void poll_destroy(poll_t *poll);

__api int poll_add(poll_t *poll, event_t *event, pollev_t attr);

__api int poll_del(poll_t *poll, event_t *event);

__api int poll_wait(poll_t *poll, pollev_t *events, int size);

#endif /* !__OSI_POLL_H */
/*!@} */
