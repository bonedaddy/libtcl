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

/*!@file osi/fiber.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __OSI_FIBER_H
# define __OSI_FIBER_H

#include <osi/conf.h>
#include <pp.h>

#ifdef HAS_UCONTEXT_H
# include <ucontext.h>
#elif defined(OS_PROVENCORE)
# include <threads/threads.h>
#endif

/*!@public
 *
 * @brief
 * The fiber structure.
 */
typedef struct osi_fib osi_fib_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `osi_fibctor'.
 */
typedef void (osi_fibfn_t)(void *arg);

/*!@public
 *
 * @brief
 * Fiber status.
 */
typedef enum osi_fibst osi_fibst_t;

/*!@public
 *
 * @brief
 * The fiber structure definition
 */
struct osi_fib {

	/*! The fiber status */
	osi_fibst_t status;

	/*! Scheduling priority */
	uint8_t priotity;

	/*! The fiber core function */
	osi_fibfn_t *fn;

	/*! The fiber core function */
	void *arg;

#ifdef HAS_UCONTEXT_H

	/*! Fiber context */
	ucontext_t context;

	/*! Fiber stack */
	void *stack;
#elif defined(OS_PROVENCORE)

	/*! Fiber PNC context */
	struct context *context;
#endif

	/*! The fiber successor */
	osi_fib_t *successor;

	/*! The fiber predecessor */
	osi_fib_t *predecessor;

	/*! The header of all the fibers */
	osi_fib_t *parent;
};

/*!@public
 *
 * @brief
 * The fiber status definition
 */
enum osi_fibst {

	/*! The fiber was just created so ready */
	OSI_FIB_READY,

	/*! The fiber is running is it's own context until it finish or yield */
	OSI_FIB_RUNNING,

	/*! The fiber is terminated but still exists */
	OSI_FIB_EXITING,
};

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `osi_sched'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fib  The fiber to construct.
 * @param fn   The fiber function.
 * @param arg  The fiber function argument.
 * @param ss   The stack size of the new fiber.
 * @param prio The priority of the new fiber.
 * @return     0 on success 1 otherwise.
 */
__api__ int osi_fibctor(osi_fib_t *, osi_fibfn_t *, void *, uint16_t, uint8_t);

/*!@public
 *
 * @brief
 * Deletes the given fiber.
 *
 * @param fib The fiber to deconstruct.
 * @return    0 on success 1 otherwise
 */
__api__ void osi_fibdtor(osi_fib_t *);

/*!@public
 *
 * @brief
 *
 * @param fib
 * @return
 */
__api__ void osi_fibjoin(osi_fib_t *);

/*!@public
 *
 * @brief
 *
 * @return
 */
__api__ void osi_sched(void);

/*!@public
 *
 * @brief
 *
 * @return
 */
__api__ void osi_yield(void);

#endif /* __OSI_FIBER_H */
/*!@} */
