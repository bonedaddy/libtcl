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

/*!@file osi/future.h
 * @author uael
 *
 * @addtogroup osi.future @{
 */
#ifndef __OSI_FUTURE_H
# define __OSI_FUTURE_H

#include "osi/sema.h"

#define FUTURE_SUCCESS ((void *)1)
#define FUTURE_FAIL ((void *)0)

/*!@public
 *
 * @brief
 * The future structure declaration.
 */
typedef struct future future_t;

/*!@public
 *
 * @brief
 * The future structure definition.
 */
struct future {

	/*! TODO. */
	bool awaitable;

	/*! TODO. */
	sema_t sema;

	/*! TODO. */
	void const *result;
};

/*!@public
 *
 * @brief
 * TODO(uael): remove it, only new/free, await should free
 *
 * @param future
 * @return
 */
__api__ int future_init(future_t *future);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param future
 * @param value
 * @return
 */
__api__ void future_immediate(future_t *future, void const *value);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param future
 * @param value
 */
__api__ void future_ready(future_t *future, void *value);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param future
 * @return
 */
__api__ void *future_await(future_t *future);

#endif /* __OSI_FUTURE_H */
/*!@} */
