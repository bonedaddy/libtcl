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

/*!!@public
 *
 * @brief
 * TODO
 *
 * @return
 */
__private__ osi_fib_t *osi_sched_entry(void);

/*!!@public
 *
 * @brief
 * TODO
 *
 * @param fib
 */
__private__ void osi_sched_ready(osi_fib_t *fib);

#endif /* __OSI_FIBER_SCHED_H */
