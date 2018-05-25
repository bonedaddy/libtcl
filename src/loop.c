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
#define LOG_TAG "osi_loop"

#include "osi/log.h"
#include "osi/loop.h"
#include "osi/sema.h"

typedef struct {
	loop_t *loop;
	sema_t start_sem;
} start_arg_t;

static void *__loop(void *context)
{
	loop_t *loop;
	start_arg_t *arg;

	arg = (start_arg_t *)context;
	loop = arg->loop;
	loop->is_running = true;
	LOG_INFO("loop started %p", loop);
	sema_post(&arg->start_sem);

	while (loop->is_running) {
		loop->work(loop->context);
#ifndef OSI_THREADING
		fiber_schedule();
#endif
	}

	LOG_INFO("loop exited %p", loop);
	return NULL;
}

int loop_init(loop_t *loop, work_t *work, void *context)
{
	start_arg_t start;

	if (sema_init(&start.start_sem, 0))
		return -1;
	start.loop = loop;
	loop->is_running = false;
	loop->work = work;
	loop->context = context;
#ifdef OSI_THREADING
	pthread_create(&loop->pthread, NULL, __loop, &start);
#else
	fiber_init(&loop->fiber, __loop, (fiber_attr_t){ .context = &start });
#endif
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	return 0;
}

void loop_destroy(loop_t *loop)
{
	loop_join(loop);
#ifndef OSI_THREADING
	fiber_destroy(loop->fiber);
#endif /* !OSI_THREADING */
}

void loop_stop(loop_t *loop)
{
	loop->is_running = false;
}

void loop_join(loop_t *loop)
{
	if (loop->is_running) {
		loop_stop(loop);
#ifdef OSI_THREADING
		//TODO(uael): ??? pthread_join(loop->pthread, NULL);
#else
		fiber_join(loop->fiber);
#endif /* OSI_THREADING */
	}
}
