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

/*!@file osi.h
 * @author uael
 */
#ifndef __OSI_H
# define __OSI_H

#include "osi/conf.h"
#include "osi/alarm.h"
#include "osi/allocator.h"
#include "osi/blocking_queue.h"
#include "osi/buffer.h"
#include "osi/dispatcher.h"
#include "osi/event.h"
#include "osi/fiber.h"
#include "osi/future.h"
#include "osi/list.h"
#include "osi/loop.h"
#include "osi/map.h"
#include "osi/module.h"
#include "osi/mutex.h"
#include "osi/poll.h"
#include "osi/properties.h"
#include "osi/queue.h"
#include "osi/reader.h"
#include "osi/reactor.h"
#include "osi/ringbuffer.h"
#include "osi/sema.h"
#include "osi/stack.h"
#include "osi/string.h"
#include "osi/thread.h"
#include "osi/vector.h"

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void osi_init(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void osi_cleanup(void);

#endif /* __OSI_H */
