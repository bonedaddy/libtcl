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

void node_init(node_t *node)
{
	node->pred = node->succ = (node_t *) node;
}

void list_init(list_t *list)
{
	list->pred = list->succ = (node_t *) list;
	list->len = 0;
}

void list_push(list_t *list, node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->succ = list->succ;
	entry->pred = (node_t *) list;
	list->succ->pred = entry;
	list->succ = entry;
	list->len++;
}

void list_unshift(list_t *list, node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->pred = list->pred;
	entry->succ = (node_t *) list;
	list->pred->succ = entry;
	list->pred = entry;
	list->len++;
}

void list_detach(list_t *list, node_t *entry)
{
	node_t *succ;
	node_t *pred;

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

node_t *list_shift(list_t *list)
{
	node_t *succ;

	if (list == NULL)
		return NULL;
	succ = list->succ;
	if (succ == (node_t *) list)
		return NULL;
	list_detach(list, succ);
	return succ;
}

node_t *list_pop(list_t *list)
{
	node_t *pred;

	if (list == NULL)
		return NULL;
	pred = list->pred;
	if (pred == (node_t *) list)
		return NULL;
	list_detach(list, pred);
	return pred;
}

bool list_contains(list_t *list, node_t *entry)
{
	node_t *head;

	head = list->succ;
	while (head != (node_t *)list) {
		if (head == entry)
			return true;
		head = head->succ;
	}
	return false;
}
