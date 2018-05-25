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

#include "osi/queue.h"

__always_inline void queue_init(queue_t *queue, size_t isize)
{
	vector_init(&queue->base, isize);
}

__always_inline void queue_destroy(queue_t *queue, queue_dtor_t *idtor)
{
	vector_destroy(&queue->base, idtor);
}

__always_inline void queue_clear(queue_t *queue, queue_dtor_t *idtor)
{
	vector_clear(&queue->base, idtor);
}

__always_inline size_t queue_length(queue_t *queue)
{
	return vector_length(&queue->base);
}

__always_inline void *queue_peek(queue_t *queue)
{
	return vector_begin(&queue->base);
}

__always_inline void queue_ensure(queue_t *queue, size_t n)
{
	vector_ensure(&queue->base, n);
}

__always_inline void queue_grow(queue_t *queue, size_t n_added)
{
	vector_grow(&queue->base, n_added);
}

__always_inline void *queue_npush(queue_t *queue, size_t n)
{
	return vector_npush_back(&queue->base, n);
}

__always_inline size_t queue_npop(queue_t *queue, size_t n, void *out)
{
	return vector_npop_front(&queue->base, n, out);
}

__always_inline void *queue_push(queue_t *queue)
{
	return queue_npush(queue, 1);
}

__always_inline bool queue_pop(queue_t *queue, void *out)
{
	return queue_npop(queue, 1, out) == 1;
}

__always_inline size_t queue_index_of(queue_t *queue, void *item) {
	return vector_index_of(&queue->base, item);
}

__always_inline bool queue_pop_at(queue_t *queue, size_t idx, void *out) {
	return vector_pop_at(&queue->base, idx, out);
}