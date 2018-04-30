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

#include <osi/list.h>

#include <stddef.h>

void osi_list_init(osi_list_t *list)
{
	list->pred = list->succ = (osi_node_t *) list;
	list->len = 0;
}

void osi_list_push(osi_list_t *list, osi_node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->succ = list->succ;
	entry->pred = (osi_node_t *) list;
	list->succ->pred = entry;
	list->succ = entry;
	list->len++;
}

void osi_list_unshift(osi_list_t *list, osi_node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->pred = list->pred;
	entry->succ = (osi_node_t *) list;
	list->pred->succ = entry;
	list->pred = entry;
	list->len++;
}

void osi_list_detach(osi_list_t *list, osi_node_t *entry)
{
	osi_node_t *succ;
	osi_node_t *pred;

	if (entry == NULL)
		return;
	succ = entry->succ;
	pred = entry->pred;
	if (succ == NULL || pred == NULL)
		return;
	pred->succ = succ;
	succ->pred = pred;
	list->len--;
	entry->succ = entry->pred = entry;
}

osi_node_t *osi_list_shift(osi_list_t *list)
{
	osi_node_t *succ;

	if (list == NULL)
		return NULL;
	succ = list->succ;
	if (succ == (osi_node_t *) list)
		return NULL;
	osi_list_detach(list, succ);
	return succ;
}

osi_node_t *osi_list_pop(osi_list_t *list)
{
	osi_node_t *pred;

	if (list == NULL)
		return NULL;
	pred = list->pred;
	if (pred == (osi_node_t *) list)
		return NULL;
	osi_list_detach(list, pred);
	return pred;
}
