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

/*!@file osi/ring.h
 * @author uael
 *
 * @addtogroup osi.ds @{
 */
#ifndef __OSI_RING_H
# define __OSI_RING_H

#include <osi/conf.h>
#include <pp.h>
#include <stdint.h>
#include <stddef.h>

typedef struct osi_ring osi_ring_t;

struct osi_ring {

	/** Successor */
	osi_ring_t *succ;

	/** Predecessor */
	osi_ring_t *pred;

	/** The header of all the rings */
	osi_ring_t *parent;

	/** The count in the ring */
	uint16_t len;
};

__private__ void osi_ring_init(osi_ring_t *ring);

__private__ void osi_ring_prepend(osi_ring_t *ring, osi_ring_t *entry);

__private__ void osi_ring_append(osi_ring_t *ring, osi_ring_t *entry);

__private__ void osi_ring_detach(osi_ring_t *entry);

__private__ osi_ring_t *osi_ring_shift(osi_ring_t *ring);

__private__ osi_ring_t *osi_ring_pop(osi_ring_t *ring);

#define RING_INITIALIZER(ring) \
	{ &(ring), &(ring), &(ring), 0 }

#define RING_ENTRY(ring_ptr, app_type, ring_member) \
    ((app_type *) (((char *) (ring_ptr)) - offsetof(app_type,ring_member)))

#endif /* __OSI_RING_H */
/*!@} */
