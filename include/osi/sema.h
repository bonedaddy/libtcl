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

/*!@file osi/sema.h
 * @author uael
 *
 * @addtogroup osi.sema @{
 */
#ifndef __OSI_SEMA_H
# define __OSI_SEMA_H

#include <osi/conf.h>
#include <osi/fiber/ev.h>

/*!@public
 *
 * @brief
 * The semaphore opaque structure.
 */
typedef struct sema sema_t;

/*!@public
 *
 * @brief
 * The sema structure definition
 */
struct sema {

	/*! fd when `OSI_THREADING', counter otherwise */
	int handle;

#ifndef OSI_THREADING

	/*! On fiber fiber mode, we use an event to delegate work. */
	fiber_ev_t ev;
#endif
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param value
 * @return
 */
__api__ int sema_init(sema_t *sema, unsigned value);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sema
 */
__api__ void sema_destroy(sema_t *sema);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sema
 */
__api__ void sema_wait(sema_t *sema);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sema
 * @return
 */
__api__ bool sema_trywait(sema_t *sema);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sema
 */
__api__ void sema_post(sema_t *sema);

#endif /* __OSI_SEMA_H */
/*!@} */
