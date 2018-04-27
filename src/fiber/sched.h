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

/*!@file fiber/sched.h
 * @author uael
 */
#ifndef __OSI_FIBER_SCHED_H
# define __OSI_FIBER_SCHED_H

#include <osi/sched.h>

#include "fiber.h"

typedef struct osi_sched osi_sched_t;

struct osi_sched {
	osi_fib_t *fibers;
	size_t size;
	size_t slot;

	osi_ring_t ready;
	osi_ring_t dead;

	osi_fib_t *running;
	osi_fib_t *root;
};

extern osi_sched_t *__scheduler;
extern int __scheduled;

__private__ void osi_sched_switch(void);

#endif /* __OSI_FIBER_SCHED_H */
