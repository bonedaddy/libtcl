/*
 * Copyright 2018 Tempow
 *
 * Author - 2018 uael <abel@tempow.com>
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

#define LOG_TAG "test/alarm"

#include "alarm.h"

int main(void)
{
	alarm_t *alarm = alarm_new("alarm_test.test_cancel");

	alarm_set(alarm, 10, cb, NULL);
	alarm_cancel(alarm);

	msleep(10 + EPSILON_MS);

	ASSERT_EQ(cb_counter, 0);
	alarm_free(alarm);
	return 0;
}