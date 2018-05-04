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

#include <osi/event.h>

#include <assert.h>
#include <stdio.h>

static equeue_t queue;
static int counter = 0;

void *reader(equeue_t *equeue) {
	event_t *ev;

	while ((ev = equeue_pop(equeue)))
		printf("READ: %s\n", ev->data);
	++counter;
	return (NULL);
}

void *writer(sched_t *sched) {

	printf("WRITE: %s\n", "Hello");
	equeue_push(&queue, ({
		event_t *ev;

		ev = malloc(sizeof(event_t) + 6);
		memcpy(ev->data, "Hello", 6);
		ev;
	}));

	printf("WRITE: %s\n", " Zob ");
	equeue_push(&queue, ({
		event_t *ev;

		ev = malloc(sizeof(event_t) + 6);
		memcpy(ev->data, " Zob ", 6);
		ev;
	}));

	printf("WRITE: %s\n", "World");
	equeue_push(&queue, ({
		event_t *ev;

		ev = malloc(sizeof(event_t) + 6);
		memcpy(ev->data, "World", 6);
		ev;
	}));

	if (counter >= 42)
		sched_stop(sched);
	return (NULL);
}

int main(void)
{
	sched_t sched;

	equeue_init(&queue);
	equeue_listen(&queue, &sched, reader);

	sched_init(&sched);
	sched_loop(&sched, (work_t *)writer, 32, &sched, 1);
	sched_start(&sched);
	assert(counter == 42);

	equeue_destroy(&queue);
	return 0;
}
