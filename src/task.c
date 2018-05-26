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
#define LOG_TAG "osi_task"

#include "osi/log.h"
#include "osi/task.h"
#include "osi/sema.h"

typedef struct {
	task_t *task;
	sema_t start_sem;
} start_arg_t;

static void *__repeat(void *context)
{
	task_t *task;
	start_arg_t *arg;

	arg = (start_arg_t *)context;
	task = arg->task;
	task->running = true;
	LOG_INFO("task repeat started %p", task);
	sema_post(&arg->start_sem);

	while (task->running) {
		task->work(task->context);
#ifndef OSI_THREADING
		fiber_schedule();
#endif
	}

	task->joined = true;
	LOG_INFO("task repeat exited %p", task);
	return NULL;
}

static void *__spawn(void *context)
{
	task_t *task;
	start_arg_t *arg;

	arg = (start_arg_t *)context;
	task = arg->task;
	task->running = true;
	LOG_INFO("task spawn started %p", task);
	sema_post(&arg->start_sem);

	task->work(task->context);
#ifndef OSI_THREADING
	fiber_schedule();
#endif

	task->running = false;
	task->joined = true;
	LOG_INFO("task spawn exited %p", task);
	return NULL;
}

static __always_inline int __init(task_t *task, work_t *work, void *context,
	bool repeat)
{
	start_arg_t start;
	work_t *task_work;

	if (sema_init(&start.start_sem, 0))
		return -1;
	start.task = task;
	task->running = false;
	task->joined = false;
	task->work = work;
	task->context = context;
	task_work = repeat ? __repeat : __spawn;
#ifdef OSI_THREADING
	if (pthread_create(&task->pthread, NULL, task_work, &start))
		return -1;
#else
	fiber_init(&task->fiber, task_work, (fiber_attr_t){ .context = &start });
#endif
	sema_wait(&start.start_sem);
	sema_destroy(&start.start_sem);
	return 0;
}

int task_spawn(task_t *task, work_t *work, void *context)
{
	return __init(task, work, context, false);
}

int task_repeat(task_t *task, work_t *work, void *context)
{
	return __init(task, work, context, true);
}

__always_inline void task_destroy(task_t *task)
{
	task_join(task);
}

__always_inline bool task_running(task_t *task)
{
	return !task->joined;
}

__always_inline int task_setpriority(task_t *task, int priority)
{
	(void)task;
	(void)priority;
	//TODO(uael)
	return 1;
}

__always_inline void task_stop(task_t *task)
{
	task->running = false;
}

__always_inline void task_join(task_t *task)
{
	if (!task->joined) {
		task_stop(task);
#ifdef OSI_THREADING
		pthread_join(task->pthread, NULL);
#else
		fiber_join(task->fiber);
#endif /* OSI_THREADING */
	}
}

__always_inline void task_schedule(void)
{
#ifdef OSI_THREADING
	sched_yield();
#else
	fiber_schedule();
#endif
}
