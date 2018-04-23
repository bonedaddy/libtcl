/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

/*!@file osi/fiber.h
 * @author uael
 */
#ifndef __TEMPOW_OSI_FIBER_H
# define __TEMPOW_OSI_FIBER_H

#include <pp.h>

typedef struct osi_fiber osi_fiber_t;

typedef void (osi_fiber_fn_t)(osi_fiber_t *fiber, void *arg);

/**
 *
 * @param fiber
 * @param fn
 * @param ss
 * @return
 */
__api__ int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss);

/**
 *
 * @param fiber
 * @return
 */
__api__ int osi_fiber_dtor(osi_fiber_t *fiber);

/**
 *
 * @param fiber
 * @return
 */
__api__ int	osi_fiber_start(osi_fiber_t *fiber, void *arg);

/**
 *
 * @param fiber
 * @return
 */
__api__ int	osi_fiber_join(osi_fiber_t *fiber);

/**
 *
 * @param fiber
 * @return
 */
__api__ int osi_fiber_yield(osi_fiber_t *fiber);

#endif /* __TEMPOW_OSI_FIBER_H */
