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

#include <osi/event.h>

/*!@public
 *
 * @brief
 * The alarm structure declaration.
 */
typedef struct alarm alarm_t;

/*!@public
 *
 * @brief
 * The alarm structure definition.
 */
struct alarm {
	const char *name;
	uint64_t last_exec;
	uint64_t interval;
	bool periodic;
	void *data;
	work_t *work;
};

#endif /* __OSI_ALARM_H */
/*!@} */
