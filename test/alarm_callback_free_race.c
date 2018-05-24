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

// Try to catch any race conditions between the timer callback and |alarm_free|.
void test_callback_free_race(void) {
	char alarm_name[50];

	strcpy(alarm_name, "alarm_test.test_callback_free_race[000]");
	for (int i = 0; i < 100; i++) {
		alarm_name[35] = (char)(i % 1000 / 100 + '0');
		alarm_name[36] = (char)(i % 100 / 10+ '0');
		alarm_name[37] = (char)(i % 10 + '0');
		alarm_t *alarm = alarm_new(alarm_name);
		alarm_set(alarm, 0, cb, NULL);
		alarm_free(alarm);
	}
	alarm_cleanup();
}
int main(void) {
	test_callback_free_race();
	return (0);
}