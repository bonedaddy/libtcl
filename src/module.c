
#define LOG_TAG "module"
#include "osi/module.h"
#include "osi/mutex.h"
#include "osi/map.h"
#include "osi/log.h"

static bool call_lifecycle_function(module_lifecycle_fn function) {
	// A NULL lifecycle function means it isn't needed, so assume success
	if (!function)
		return true;

	future_t future;

	if (!function(&future))
		return true;

	// Otherwise fall back to the future
	return (bool)future_await(&future);
}

bool module_init(module_t *module) {
#ifdef OSI_LOGGING
	static int nested = 0;
#endif

	assert(module != NULL);
	if ((module->state & MODULE_STATE_INITIALIZED))
	{
		LOG_WARN("Module \"%s\" Already initialized", module->name);
		return true;
	}
	ITER_OVER_DEPENDENCIES(module_init);
	LOG_INFO("Initializing module \"%s\" (nested %d)", module->name, nested);
	if (!call_lifecycle_function(module->init)) {
		LOG_ERROR("Failed to initialize module \"%s\"", module->name);
		return false;
	}
	LOG_INFO("Initialized module \"%s\"", module->name);
	module->state |= MODULE_STATE_INITIALIZED;
	return true;
}

bool module_start_up(module_t *module) {
#ifdef OSI_LOGGING
	static int nested = 0;
#endif

	assert(module != NULL);
	if ((module->state & MODULE_STATE_STARTED)) {
		LOG_WARN("Module \"%s\" Already started", module->name);
		return true;
	}
	if (!(module->state & MODULE_STATE_INITIALIZED) && !module_init(module))
		return false;

	ITER_OVER_DEPENDENCIES(module_start_up);
	LOG_INFO("Starting module \"%s\" (nested %d)", module->name, nested); \
	if (!call_lifecycle_function(module->start_up)) {
		LOG_ERROR("Failed to start up module \"%s\"", module->name);
		return false;
	}
	LOG_INFO("Started module \"%s\"", module->name);
	module->state |= MODULE_STATE_STARTED;
	return true;
}

bool module_shut_down(module_t *module) {
#ifdef OSI_LOGGING
	static int nested = 0;
#endif

	assert(module != NULL);
	if (!(module->state & MODULE_STATE_STARTED)) {
		LOG_WARN("Module \"%s\" was not started", module->name);
		return true;
	}
	LOG_INFO("Shutting down module \"%s\" (nested %d)", module->name, nested);
	if (!call_lifecycle_function(module->shut_down)) {
		LOG_ERROR("Failed to shutdown module \"%s\".", module->name);
		return false;
	}
	LOG_INFO("Shutdown of module \"%s\" completed", module->name);
	ITER_OVER_DEPENDENCIES(module_shut_down);
	module->state &= ~(MODULE_STATE_STARTED);
	return (true);
}


bool module_clean_up(module_t *module) {
#ifdef OSI_LOGGING
	static int nested = 0;
#endif

	assert(module != NULL);
	if ((module->state & MODULE_STATE_NONE)) {
		LOG_WARN("Module \"%s\" Already clean up", module->name);
		return true;
	}
	if ((module->state & MODULE_STATE_STARTED) && !module_shut_down(module)) {
		return false;
	}

	LOG_INFO("Cleaning up module \"%s\"", module->name);
	if (!call_lifecycle_function(module->clean_up)) {
		LOG_ERROR("Failed to cleanup module \"%s\".", module->name);
		return false;
	}
	LOG_INFO("Cleanup of module \"%s\" completed", module->name);
	ITER_OVER_DEPENDENCIES(module_clean_up);
	module->state &= ~(MODULE_STATE_INITIALIZED);
	return true;
}
