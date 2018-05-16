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

/*!@file osi/fiber/ev.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __OSI_FIBER_EV_H
# define __OSI_FIBER_EV_H

#include <osi/conf.h>
#include <osi/list.h>

/*!@private
 *
 * @brief
 * The fiber event structure declaration.
 */
typedef struct fiber_ev fiber_ev_t;

/*!@private
 *
 * @brief
 * Possible fiber event kind definition.
 */
typedef enum fiber_evk fiber_evk_t;

/*!@private
 *
 * @brief
 * Possible fiber event kind definition.
 */
enum fiber_evk {

	/*! Manual reset event. */
	FIBER_EV_MANUAL,

	/*! Auto-reset event. */
	FIBER_EV_AUTO,
};

/*!@private
 *
 * @brief
 * The fiber event structure declaration.
 */
struct fiber_ev {

	/*! The fiber event pending list. */
	list_t pending;

	/*! The fiber event kind. */
	fiber_evk_t kind;

	/*! If the event is set or not. */
	bool isset;
};

/*!@private
 *
 * @brief
 * Creates the event with the given type.
 * The newly created event is in the reset state.
 * Always returns correct result.
 *
 * @param ev The fiber event to initialize.
 */
__api__ void fiber_ev_init(fiber_ev_t *ev, fiber_evk_t kind);

/*!@private
 *
 * @brief
 * Deletes the event.
 *
 * @param ev The fiber event to destroy.
 */
__api__ void fiber_ev_destroy(fiber_ev_t *ev);

/*!@private
 *
 * @brief
 * Set the given fiber event.
 *
 * @param ev The fiber event to set.
 */
__api__ void fiber_ev_set(fiber_ev_t *ev);

/*!@private
 *
 * @brief
 * Reset the given fiber event.
 *
 * @param ev The fiber event to reset.
 */
__api__ void fiber_ev_reset(fiber_ev_t *ev);

/*!@private
 *
 * @brief
 * Waits while the given fiber event will be set.
 *
 * @param ev The fiber event to wait for.
 */
__api__ void fiber_ev_wait(fiber_ev_t *ev);

/*!@private
 *
 * @brief
 * Returns false if event isn't set, otherwise returns true.
 */
__api__ bool fiber_ev_isset(fiber_ev_t *ev);

#endif /* __OSI_FIBER_EV_H */
/*!@} */
