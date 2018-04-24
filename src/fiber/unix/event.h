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

/*!@file fiber/unix/event.h
 * @author uael
 *
 * @addtogroup osi.fiber.event @{
 */
#ifndef __OSI_FIBER_UNIX_EVENT_H
# define __OSI_FIBER_UNIX_EVENT_H

#include "fiber/unix/internal.h"

#ifdef FIBER_UNIX
# include <pp.h>

/*!@private
 *
 * @brief
 * The opaque event structure.
 */
typedef struct osi_event osi_event_t;

/*!@private
 *
 * @brief
 * The kind of event.
 */
typedef enum {
	OSI_EVENT_MANUAL,
	OSI_EVENT_AUTO,
} osi_eventk_t;

/*!@private
 *
 * @brief
 * Populate an event.
 *
 * @param event The event to construct.
 * @param kind  The kind of the event.
 */
__private__ void osi_event_ctor(osi_event_t *event, osi_eventk_t kind);

/*!@private
 *
 * @brief
 * Deconstruct an event.
 *
 * @param event The event to deconstruct.
 */
__private__ void osi_event_dtor(osi_event_t *event);

/*!@private
 *
 * @brief
 * Set an event.
 *
 * @param event The event to set.
 */
__private__ void osi_event_set(osi_event_t *event);

/*!@private
 *
 * @brief
 * Reset an event.
 *
 * @param event The event to reset.
 */
__private__ void osi_event_reset(osi_event_t *event);

/*!@private
 *
 * @brief
 * Check if an event is set.
 *
 * @param event The event to check.
 * @return      1 if is set, 0 otherwise.
 */
__private__ int osi_event_isset(osi_event_t *event);

/*!@private
 *
 * @brief
 * Wait for an event to finish.
 *
 * @param event The event to wait for.
 */
__private__ void osi_event_wait(osi_event_t *event);

#endif /* FIBER_UNIX */

#endif /* __OSI_FIBER_EVENT_H */
/*!@} */
