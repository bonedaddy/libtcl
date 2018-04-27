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

/*!@file osi/sched.h
 * @author uael
 *
 * @addtogroup osi.sched @{
 */
#ifndef __OSI_SCHED_H
# define __OSI_SCHED_H

#include <osi/fiber.h>

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void osi_schedule(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void osi_yield(void);

#endif /* __OSI_FIBER_H */
/*!@} */
