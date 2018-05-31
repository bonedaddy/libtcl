/*
 * Copyright (C) 2014 Google, Inc.
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

#pragma once

/*!@file osi/log.h
 * @author uael
 *
 * @addtogroup osi.log @{
 */
#ifndef __OSI_LOG_H
#define __OSI_LOG_H

#include "osi/conf.h"
#include <stdio.h>

#if defined(OSI_LOGGING)
# if !defined(LOG_TAG)
#   warning "LOG_TAG must be defined"
#   define LOG_TAG __FILE__
# endif /* !defined(LOG_TAG) */
# if defined(OS_PROVENCORE)
#   define APP_NAME_LO LOG_TAG
#   include_next <log.h>
#   define EXPEND_LOG_ARGS(fmt, args...) "%s: " fmt "\n", LOG_TAG, ##args
#   define LOG_VERBOSE(...) printf(EXPEND_LOG_ARGS(__VA_ARGS__))
#   define LOG_DEBUG(...) pr_debug(EXPEND_LOG_ARGS(__VA_ARGS__))
#   define LOG_INFO(...) pr_info(EXPEND_LOG_ARGS(__VA_ARGS__))
#   define LOG_WARN(...) pr_warn(EXPEND_LOG_ARGS(__VA_ARGS__))
#   define LOG_ERROR(...) pr_err(EXPEND_LOG_ARGS(__VA_ARGS__))
# else /* !defined(OS_PROVENCORE) */
#   define LOGWRAP(lvl, fmt, args...) \
		fprintf(stdout, "[%s] %s: %s: %d "fmt"\n", #lvl, LOG_TAG, __func__, \
			__line__, ##args)
#   ifdef NDEBUG
#     define LOG_VERBOSE(...) do; while (0)
#   else
#     define LOG_VERBOSE(fmt, args...) LOGWRAP(LOG_VERBOSE, fmt, ##args)
#   endif /* NDEBUG */
#   define LOG_DEBUG(fmt, args...) LOGWRAP(LOG_DEBUG, fmt, ## args)
#   define LOG_INFO(fmt, args...) LOGWRAP(LOG_INFO, fmt, ## args)
#   define LOG_WARN(fmt, args...) LOGWRAP(LOG_WARN, fmt, ## args)
#   define LOG_ERROR(fmt, args...) LOGWRAP(LOG_ERROR, fmt, ## args)
# endif /* defined(OS_PROVENCORE) */
#else /* !defined(OSI_LOGGING) */
# define LOG_VERBOSE(fmt, args...) do {} while (0)
# define LOG_DEBUG(fmt, args...) do {} while (0)
# define LOG_INFO(fmt, args...) do {} while (0)
# define LOG_WARN(fmt, args...) do {} while (0)
# define LOG_ERROR(fmt, args...) do {} while (0)
#endif /* defined(OSI_LOGGING) */

#endif
