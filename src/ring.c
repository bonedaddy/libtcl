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

#include <osi/ring.h>

#include <stddef.h>

void osi_ring_init(osi_ring_t *ring)
{
	ring->pred = ring->succ = ring;
	ring->parent = ring;
	ring->len = 0;
}

void osi_ring_append(osi_ring_t *ring, osi_ring_t *entry)
{
	if (ring == NULL || entry == NULL)
		return;
	entry->succ = ring->succ;
	entry->pred = ring;
	entry->parent = ring->parent;
	ring->succ->pred = entry;
	ring->succ = entry;
	ring->parent->len++;
}

void osi_ring_prepend(osi_ring_t *ring, osi_ring_t *entry)
{
	if (ring == NULL || entry == NULL)
		return;
	entry->pred = ring->pred;
	entry->succ = ring;
	entry->parent = ring->parent;
	ring->pred->succ = entry;
	ring->pred = entry;
	ring->parent->len++;
}

void osi_ring_detach(osi_ring_t *entry)
{
	osi_ring_t *succ;
	osi_ring_t *pred;

	if (entry == NULL || entry->parent == entry)
		return;
	succ = entry->succ;
	pred = entry->pred;
	if (succ == NULL || pred == NULL)
		return;
	pred->succ = succ;
	succ->pred = pred;
	entry->parent->len--;
	entry->succ = entry->pred = entry;
	entry->parent = entry;
	entry->len = 0;
}

osi_ring_t *osi_ring_shift(osi_ring_t *ring)
{
	osi_ring_t *succ;

	if (ring == NULL)
		return NULL;
	succ = ring->succ;
	if (succ == ring)
		return NULL;
	osi_ring_detach(succ);
	return succ;
}

osi_ring_t *osi_ring_pop(osi_ring_t *ring)
{
	osi_ring_t *pred;

	if (ring == NULL)
		return NULL;
	pred = ring->pred;
	if (pred == ring)
		return NULL;
	osi_ring_detach(pred);
	return pred;
}
