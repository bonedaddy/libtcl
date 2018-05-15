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
	node->prev = node->next = node;
}

void list_init(list_t *list)
{
	list->prev = list->next = (node_t *) list;
	list->len = 0;
}

void list_destroy(list_t *list, node_dtor_t *dtor)
{
	node_t *head;

	while ((head = list_pop(list)))
		if (dtor) dtor(head);
}

bool list_empty(list_t *list)
{
	return list->len == 0;
}

void list_push(list_t *list, node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->next = list->next;
	entry->prev = (node_t *) list;
	list->next->prev = entry;
	list->next = entry;
	list->len++;
}

void list_unshift(list_t *list, node_t *entry)
{
	if (list == NULL || entry == NULL)
		return;
	entry->prev = list->prev;
	entry->next = (node_t *) list;
	list->prev->next = entry;
	list->prev = entry;
	list->len++;
}

void list_detach(list_t *list, node_t *entry)
{
	node_t *succ;
	node_t *pred;

	if (entry == NULL)
		return;
	succ = entry->next;
	pred = entry->prev;
	if (succ == NULL || pred == NULL)
		return;
	pred->next = succ;
	succ->prev = pred;
	list->len--;
	entry->next = entry->prev = entry;
}

node_t *list_shift(list_t *list)
{
	node_t *succ;

	if (list == NULL)
		return NULL;
	succ = list->next;
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
	pred = list->prev;
	if (pred == (node_t *) list)
		return NULL;
	list_detach(list, pred);
	return pred;
}

bool list_contains(list_t *list, node_t *entry)
{
	node_t *head;

	head = list->next;
	while (head != (node_t *)list) {
		if (head == entry)
			return true;
		head = head->next;
	}
	return false;
}
