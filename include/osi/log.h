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

#pragma once

/*!@file osi/log.h
 * @author uael
 *
 * @addtogroup osi.log @{
 */
#ifndef __OSI_LOG_H
# define __OSI_LOG_H

#include <osi/conf.h>
#include <stdio.h>

#ifdef OSI_LOGGING
# ifdef OS_PROVENCORE
#   define APP_NAME_LO LOG_TAG
#   include_next <log.h>
#   define LOG_VERBOSE(TAG, fmt, args...) pr_info(fmt, ## args)
#   define LOG_DEBUG(TAG, fmt, args...) pr_debug(fmt, ## args)
#   define LOG_INFO(TAG, fmt, args...) pr_info(fmt, ## args)
#   define LOG_WARN(TAG, fmt, args...) pr_warn(fmt, ## args)
#   define LOG_ERROR(TAG, fmt, args...) pr_err(fmt, ## args)
#   define LOGWRAP(lvl, fmt, args...) \
	lvl(fmt, ## args)
# else
#  ifdef NDEBUG
#   define LOG_VERBOSE(...) do; while (0)
#  else
#   define LOG_VERBOSE(fmt, args...) LOGWRAP(LOG_VERBOSE, fmt, ##args)
#  endif /* NDEBUG */
#  define LOGWRAP(lvl, fmt, args...) \
	fprintf(stderr, "[%s] %s:%s: "fmt"\n", #lvl, LOG_TAG, __func__, ##args)
#  define LOG_DEBUG(fmt, args...) LOGWRAP(LOG_DEBUG, fmt, ## args)
#  define LOG_INFO(fmt, args...) LOGWRAP(LOG_INFO, fmt, ## args)
#  define LOG_WARN(fmt, args...) LOGWRAP(LOG_WARN, fmt, ## args)
#  define LOG_ERROR(fmt, args...) LOGWRAP(LOG_ERROR, fmt, ## args)
# endif
#endif /* OSI_LOGGING */

#endif /* __OSI_LOG_H */
/*!@} */
