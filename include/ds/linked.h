#pragma once

#include <stddef.h>

#include "generic.h"

struct slist_node {
	struct slist_node *next;
};

struct dlist_node {
	struct dlist_node *next, *prev;
};

#define slist_entry(ptr, type, member) container_of(ptr, type, member)

#define dlist_entry(ptr, type, member) container_of(ptr, type, member)

#define slist_get_next(ptr, member) ({ \
	const struct slist_node *__next = (ptr)->member.next; \
	__next == NULL ? NULL : slist_entry(__next, typeof(*(ptr)), member); })

#define dlist_get_next(ptr, member) ({ \
	const struct dlist_node *__next = (ptr)->member.next; \
	__next == NULL ? NULL : dlist_entry(__next, typeof(*(ptr)), member); })

#define dlist_get_prev(ptr, member) ({ \
	const struct dlist_node *__prev = (ptr)->member.prev; \
	__prev == NULL ? NULL : dlist_entry(__prev, typeof(*(ptr)), member); })

#define slist_set_next(ptr, member, next_entry) ({ \
	const typeof(next_entry) __next_entry = (next_entry); \
	struct slist_node *__head = &(ptr)->member; \
	struct slist_node *__next = (__next_entry == NULL) ? NULL : &__next_entry->member; \
	__head->next = __next; })

#define dlist_set_next(ptr, member, next_entry)  ({ \
	const typeof(next_entry) __next_entry = (next_entry); \
	struct dlist_node *__head = &(ptr)->member; \
	struct dlist_node *__next = (__next_entry == NULL) ? NULL : &__next_entry->member; \
	__head->next = __next; \
	if (__next != NULL) __next->prev = __head; })

#define dlist_set_prev(ptr, member, prev_entry) ({ \
	const typeof(prev_entry) __prev_entry = (prev_entry); \
	struct dlist_node *__head = &(ptr)->member; \
	struct dlist_node *__prev = (__prev_entry == NULL) ? NULL : &__prev_entry->member; \
	__head->prev = __prev; \
	if (__prev != NULL) __prev->next = __head; })

#define slist_foreach(head, member, start) \
	for (typeof(start) (head) = start; (head) != NULL; \
		(head) = slist_get_next((head), member))

#define dlist_foreach(head, member, start) \
	for (typeof(start) (head) = start; (head) != NULL; \
		(head) = dlist_get_next((head), member))

#define slist_append(head, member, node) ({ \
	struct slist_node *__head = &(head)->member; \
	struct slist_node *__node = &(node)->member; \
	__slist_append(__head, __node); })

#define dlist_append(head, member, node) ({ \
	struct dlist_node *__head = &(head)->member; \
	struct dlist_node *__node = &(node)->member; \
	__dlist_append(__head, __node); })

#define dlist_remove(head, member) ({ \
	struct dlist_node *__head = &(head)->member; \
	__head->prev->next = __head->next; })

static inline void __slist_append(struct slist_node *head, struct slist_node *node) {
	struct slist_node *prev = head;
	while (head != NULL)
		prev = head, head = head->next;
	prev->next = node;
}

static inline void __dlist_append(struct dlist_node *head, struct dlist_node *node) {
	struct dlist_node *prev = head;
	while (head != NULL)
		prev = head, head = head->next;
	prev->next = node;
	if (node != NULL)
		node->prev = prev;
}
