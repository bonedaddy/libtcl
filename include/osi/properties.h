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

/*!@file osi/properties.h
 * @author uael
 *
 * @addtogroup osi.properties @{
 */
#ifndef __OSI_PROPERTIES_H
# define __OSI_PROPERTIES_H

#include "osi/conf.h"

/*!@public
 *
 * @brief
 * Get value associated with key |key| into |value|.
 * Returns the length of the value which will never be greater than
 * PROPERTY_VALUE_MAX - 1 and will always be zero terminated.
 * (the length does not include the terminating zero).
 * If the property read fails or returns an empty value, the |default_value|
 * is used (if nonnull).
 *
 * @param key
 * @param value
 * @param default_value
 * @return
 */
int property_get(const char *key, char *value, const char *default_value);

/*!@public
 *
 * @brief
 * Write value of property associated with key |key| to |value|.
 * Returns 0 on success, < 0 on failure
 *
 * @param key
 * @param value
 * @return
 */
int property_set(const char *key, const char *value);

#endif /* !__OSI_PROPERTIES_H */
/*!@} */
