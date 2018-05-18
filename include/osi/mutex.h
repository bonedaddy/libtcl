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

/*!@file osi/mutex.h
 * @author uael
 *
 * @addtogroup osi.mutex @{
 */
#ifndef __OSI_MUTEX_H
# define __OSI_MUTEX_H

#include "osi/conf.h"
#include "osi/list.h"
#include "osi/fiber.h"

/*!@public
 *
 * @brief
 * The mutex structure declaration.
 */
typedef struct mutex mutex_t;

/*!@public
 *
 * @brief
 * The mutex structure definition.
 */
struct mutex {

#ifdef OSI_THREADING

	pthread_mutex_t mutex;
#else

	list_t queue;

	bool islocked;
#endif /* OSI_THREADING */
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param mutex
 * @return
 */
__api__ int mutex_init(mutex_t *mutex);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param mutex
 */
__api__ void mutex_destroy(mutex_t *mutex);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param mutex
 */
__api__ void mutex_lock(mutex_t *mutex);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param mutex
 * @return
 */
__api__ void mutex_unlock(mutex_t *mutex);

/*!@public
 *
 * @brief
 * TODO
 *
 * @return
 */
__api__ int mutex_global_init(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void mutex_global_cleanup(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void mutex_global_lock(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void mutex_global_unlock(void);

#endif /* __OSI_MUTEX_H */
/*!@} */
