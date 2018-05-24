/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#define LOG_TAG "test/alarm"
#include "alarm.h"

void test_set_short_periodic(void) {
	alarm_t *alarm = alarm_new_periodic("alarm_test.test_set_short_periodic");

	alarm_set(alarm, 10, cb, NULL);

	ASSERT_EQ(cb_counter, 0);

	for (int i = 1; i <= 10; i++) {
		sema_wait(&semaphore);

		ASSERT_GE(cb_counter, i);
	}
	alarm_cancel(alarm);

	alarm_free(alarm);
}

int main(void) {
	sema_init(&semaphore, 0);
	test_set_short_periodic();
	return (0);
}