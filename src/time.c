
#define LOG_TAG "time"

#include "osi/time.h"
#include "osi/log.h"

period_ms_t now(void) {
	struct timespec ts;

	if (clock_gettime(CLOCK_ID, &ts) == -1) {
		LOG_ERROR("Unable to get current time: %m");
		return 0;
	}
	return (period_ms_t) ((ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL));
}