/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

#include "osi/conf.h"
#include "osi/fiber.h"

#ifndef HAS_FIBER
# if defined OS_PROVENCORE
#   define FIBER_PNC
# else
#   define FIBER_NATIVE
# endif
#endif