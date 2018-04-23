/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

/*!@file osi/fiber.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __TEMPOW_OSI_FIBER_H
# define __TEMPOW_OSI_FIBER_H

#include <pp.h>

/*!@public
 *
 * @brief
 * The opaque fiber structure.
 */
typedef struct osi_fiber osi_fiber_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `osi_fiber_ctor'.
 */
typedef void (osi_fiber_fn_t)(osi_fiber_t *fiber, void *arg);

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fiber_func after
 * calling the `osi_fiber_start'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fiber The fiber to construct.
 * @param fn    The fiber function.
 * @param ss    The stack size of the new fiber.
 * @return      0 on success 1 otherwise
 */
__api__ int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss);

/*!@public
 *
 * @brief
 * Deletes the given fiber.
 *
 * @param fiber The fiber to deconstruct.
 * @return      0 on success 1 otherwise
 */
__api__ int osi_fiber_dtor(osi_fiber_t *fiber);

/*!@public
 *
 * @brief
 * Schedules the given fiber for execution and passes the ctx to the fiber func,
 * which has been set in the `osi_fiber_ctor'.
 *
 * @param fiber The fiber to start.
 * @return      0 on success 1 otherwise
 */
__api__ int	osi_fiber_start(osi_fiber_t *fiber, void *arg);

/*!@public
 *
 * @param fiber
 * @return
 */
__api__ int	osi_fiber_join(osi_fiber_t *fiber);

/*!@public
 *
 * @param fiber
 * @return
 */
__api__ int osi_fiber_yield(osi_fiber_t *fiber);

#endif /* __TEMPOW_OSI_FIBER_H */
/*!@} */
