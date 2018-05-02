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

/*!@file osi/thread.h
 * @author uael
 *
 * @addtogroup osi.thread @{
 */
#ifndef __OSI_THREAD_H
# define __OSI_THREAD_H

#include <osi/conf.h>
#include <pp.h>

/*!@public
 *
 * @brief
 * The thread opaque structure.
 */
typedef struct thread thread_t;

/*!@public
 *
 * @brief
 * TODO
 *
 * @param name
 * @return
 */
__api__ thread_t *thread_new(char const *name);


__api__ thread_t *thread_new_sized(char const *name);

#endif /* __OSI_THREAD_H */
/*!@} */
