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

// Test whether the callbacks are involed in the expected order on a
// separate queue.
void test_callback_ordering_on_queue(void) {
	alarm_t *alarms[100];
	blocking_queue_t queue;
	blocking_queue_init(&queue, UINT32_MAX);
	thread_t thread;
	thread_init(&thread , "timers.test_callback_ordering_on_queue.thread");

	alarm_register_processing_queue(&queue, &thread);

	char alarm_name[50];

	strcpy(alarm_name, "alarm_test.test_callback_ordering_on_queue[000]");
	for (int i = 0; i < 100; i++) {
		alarm_name[43] = (char)(i % 1000 / 100 + '0');
		alarm_name[44] = (char)(i % 100 / 10+ '0');
		alarm_name[45] = (char)(i % 10 + '0');
		alarms[i] = alarm_new(alarm_name);
	}

	for (int i = 0; i < 100; i++) {
		alarm_set_on_queue(alarms[i], 100, ordered_cb, INT_TO_PTR(i), &queue);
	}

	for (int i = 1; i <= 100; i++) {
		sema_wait(&semaphore);
		ASSERT_GE(cb_counter, i);
	}
	ASSERT_EQ(cb_counter, 100);
	ASSERT_EQ(cb_misordered_counter, 0);

	for (int i = 0; i < 100; i++)
		alarm_free(alarms[i]);

	alarm_unregister_processing_queue(&queue);
	blocking_queue_destroy(&queue, NULL);
	thread_destroy(&thread);
}

int main(void) {
	sema_init(&semaphore, 0);
	test_callback_ordering_on_queue();
	return (0);
}