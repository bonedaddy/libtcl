/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

#include "fiber.h"

#if defined FIBER_PNC
# include <errno.h>
# include <string.h>

# include <strings.h>
# include <threads/threads.h>

struct osi_fiber {
	osi_fiber_fn_t *fn;
	uint16_t stack_size;
	struct context *ctx;
	void *arg;
};

int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss)
{
	bzero(fiber, sizeof(osi_fiber_t));
	fiber->fn = fn;
	fiber->stack_size = ss;
	return 0;
}

int osi_fiber_dtor(osi_fiber_t *fiber)
{
	(void)fiber;
	return 0;
}

static int run(osi_fiber_t *fiber)
{
	fiber->fn(fiber, fiber->arg);
	return 0;
}

int	osi_fiber_start(osi_fiber_t *fiber, void *arg)
{
	fiber->arg = arg;
	fiber->ctx = create_context(fiber->stack_size, 0, 0, 0,
		(int (*)(void *)) run, fiber);
	return fiber->ctx == NULL;
}

int	osi_fiber_join(osi_fiber_t *fiber)
{
	if (!fiber->ctx) {
		errno = EINVAL;
		return -1;
	}
	resume(fiber->ctx, NULL);
	return 0;
}

int osi_fiber_yield(osi_fiber_t *fiber)
{
	(void)fiber;
	yield();
	return 0;
}
#endif /* defined FIBER_PNC */
