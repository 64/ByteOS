#pragma once

#include "ds/linked.h"

struct hlist_bucket {
	struct dlist_node *head;
};

#define hlist_foreach(cur, type, member, bucket) \
	for (type *cur = container_of((bucket)->head, type, member); \
		(cur) != NULL; (cur) = dlist_get_next(cur, member))

static inline void hlist_add(struct hlist_bucket *bucket, struct dlist_node *node)
{
	// Insert at front
	node->prev = NULL;
	node->next = bucket->head;

	if (bucket->head != NULL)
		bucket->head->prev = node;

	bucket->head = node;
}

static inline void hlist_remove(struct hlist_bucket *bucket, struct dlist_node *node)
{
	if (node->prev == NULL)
		bucket->head = node->next;
	else
		node->prev->next = node->next;

	// If we're the last element in the list
	if (node->next != NULL)
		node->next->prev = node->prev;
}
