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

/*!@file fiber/fiber.h
 * @author uael
 */
#ifndef __OSI_FIBER_FIBER_H
# define __OSI_FIBER_FIBER_H

#include <osi/fiber.h>
#include <osi/list.h>

#ifdef OS_PROVENCORE
# include <threads/threads.h>
#else
# include <coro.h>
#endif

/*!@private
 *
 * @brief
 * Fiber status.
 */
typedef enum fiber_st fiber_st_t;

/*!@private
 *
 * @brief
 * The fiber status definition
 */
enum fiber_st {

	/*! The fiber was just created so ready */
	OSI_FIB_READY,

	/*! The fiber is running in it's own context until it finish or yield */
	OSI_FIB_RUNNING,

	/*! The fiber is terminated but still exists */
	OSI_FIB_EXITING
};

/*!@private
 *
 * @brief
 * The fiber structure definition
 */
struct fiber {

	/*! The fiber status */
	fiber_st_t status;

	/*! The fiber core function */
	fiber_fn_t *fn;

	/*! The fiber core function argument */
	void *arg;

	/*! The fiber result */
	void *result;

	/*! The fiber caller */
	fiber_t *caller;

	/*! Fiber list hold */
	node_t hold;

	/*! The priority used by scheduler */
	int priority;

#ifdef OS_PROVENCORE
	struct context *context;
#else
	/** Coroutine context */
	coro_context context;

	/** Coroutine stack */
	struct coro_stack stack;
#endif
};

extern fiber_t *__fiber;

#endif /* __OSI_FIBER_FIBER_H */
