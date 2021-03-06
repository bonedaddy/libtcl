/*
 * Copyright (C) 2018 Tempow
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "test_alarm_set_short"

#include "alarm.h"

int main(void)
{
	alarm_t alarm;

	alarm_init(&alarm, "alarm_test.test_set_short");

	sema_init(&semaphore, 0);
	alarm_set(&alarm, 10, cb, NULL);

	ASSERT_EQ(cb_counter, 0);

	sema_wait(&semaphore);

	ASSERT_EQ(cb_counter, 1);

	alarm_destroy(&alarm);
	return 0;
}
