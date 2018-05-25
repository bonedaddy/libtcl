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

/*!@file osi/loop.h
 * @author uael
 *
 * @addtogroup osi.loop @{
 */
#ifndef __OSI_LOOP_H
# define __OSI_LOOP_H

#include "osi/fiber.h"

/*!@public
 *
 * @brief
 * The loop structure declaration.
 */
typedef struct loop loop_t;

/*!@public
 *
 * @brief
 * The loop structure definition.
 */
struct loop {

	bool is_running;

	work_t *work;

	void *context;

#ifdef OSI_THREADING

	pthread_t pthread;
#else

	fid_t fiber;
#endif
};

__api__ int loop_init(loop_t *loop, work_t *work, void *context);

__api__ void loop_destroy(loop_t *loop);

__api__ void loop_stop(loop_t *loop);

__api__ void loop_join(loop_t *loop);

#endif /* __OSI_THREAD_H */
/*!@} */
