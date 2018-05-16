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

/*!@file osi/string.h
 * @author uael
 *
 * @addtogroup osi.string @{
 */
#ifndef __OSI_STRING_H
# define __OSI_STRING_H

#include "osi/conf.h"

#ifdef OS_PROVENCORE
# include <string.h>
# include <strings.h>
#endif

//TODO TEMPOW
size_t strlcpy(char * restrict dst, const	char * restrict	src, size_t dstsize);
char *strncpy(char *dest, const char *src, size_t n);

#endif /* __OSI_STRING_H */
/*!@} */
