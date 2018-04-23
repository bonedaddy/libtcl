/*
 * Useful header with some license information and/or other stuff
 * The line 2 of this useful header
 * A third to make it real
 * Please stop reading then
 * Here we go, i warn you..
 */

#include "fiber.h"

#ifdef FIBER_NATIVE
#include <ucontext.h>

struct osi_fiber
{
	void *stack;
	ucontext_t context;
};

static osi_fiber_t main_fiber = { };
static osi_fiber_t *current_fiber = &main_fiber;

int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss)
{
	bzero(fiber, sizeof(osi_fiber_t));
	fiber->stack = calloc(ss, sizeof(char));
	getcontext(&fiber->context);
	fiber->context.uc_stack.ss_sp = fiber->stack;
	fiber->context.uc_stack.ss_size = ss;
	fiber->context.uc_link = NULL;
	makecontext(&fiber->context, (void (*)()) fn, 1, fiber);
	return 0;
}

int osi_fiber_dtor(osi_fiber_t *fiber)
{
	assert(fiber != current_fiber);
	assert(fiber != &main_fiber);

	free(fiber->stack);
	return 0;
}

void osi_fiber_switch(osi_fiber_t *fiber)
{
	ucontext_t *current_context;

	current_context = &current_fiber->context;
	current_fiber = fiber;
	swapcontext(current_context, &fiber->context);
}
#endif
