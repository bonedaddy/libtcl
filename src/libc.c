/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

#include "osi/conf.h"

#ifndef HAS_BZERO
# include <sizes.h>
# include <string.h>

void bzero(void *ptr, size_t n) {
	memset(ptr, 0, n);
}
#endif /* HAS_BZERO */
