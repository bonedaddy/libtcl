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
typedef struct osi_fiber_hdr osi_fiber_hdr_t;

typedef void (osi_fiber_fn_t)(osi_fiber_t *fiber, void *arg);

struct osi_fiber_hdr {
	char const *name;
	osi_fiber_fn_t *func;
};

/**
 *
 * @param fiber
 * @param name
 * @param func
 * @param priority
 * @param stack_size
 * @return
 */
__api__ int
osi_fiber_ctor(osi_fiber_t *fiber,
	char const *name,
	osi_fiber_fn_t *func,
	uint8_t priority,
	uint16_t stack_size);

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
__api__ int	osi_fiber_resume(osi_fiber_t *fiber);

/**
 *
 * @param fiber
 * @return
 */
__api__ int osi_fiber_yiled(osi_fiber_t *fiber);

#endif /* __TEMPOW_OSI_FIBER_H */
