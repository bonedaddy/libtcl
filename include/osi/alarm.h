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

/*!@file osi/alarm.h
 * @author uael
 *
 * @addtogroup osi.alarm @{
 */
#ifndef __OSI_ALARM_H
# define __OSI_ALARM_H

#include <osi/conf.h>
#include <pp.h>

/*!@public
 *
 * @brief
 * The alarm opaque structure.
 */
typedef struct alarm alarm_t;

/*!@public
 *
 * @brief
 * TODO
 */
typedef void (*alarm_fn_t)(void *data);

/*!@public
 *
 * @brief
 * TODO
 */
typedef uint64_t period_t;

/*!@public
 *
 * @brief
 * TODO
 *
 * @param name
 * @return
 */
__api__ alarm_t *alarm_new(char const *name);


/*!@public
 *
 * @brief
 * TODO
 *
 * @param name
 * @return
 */
__api__ alarm_t *alarm_new_periodic(char const *name);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param alarm
 */
__api__ void alarm_free(alarm_t *alarm);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param alarm
 * @param ms
 * @param cb
 * @param data
 */
__api__ void alarm_set(alarm_t *alarm, period_t ms, alarm_fn_t cb, void *data);

__api__ void alarm_set_on_queue(alarm_t *alarm, period_t interval_ms,
	alarm_fn_t cb, void *data,
	fixed_queue_t *queue);

#endif /* __OSI_ALARM_H */
/*!@} */
