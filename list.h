#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * linked list to contain dynamically allocated objects
 */

#define __list_push(name)      __list_## name ##_push
#define __list_find(name)      __list_## name ##_find
#define __list_pop(name)       __list_## name ##_pop
#define __list_new(name)       __list_## name ##_new
#define __list_free(name)      __list_## name ##_free
#define __list_purge(name)      __list_## name ##_purge

#define list_node(list) __typeof(*list->head)
#define list_data(list) __typeof(*list->head->data)

#define list_push(list, item) list->push(list, item)
#define list_pop(list, item) list->pop(list, item)
#define list_find(list, what) list->find(list, what)
#define list_new(name, constr, dest, cmp) __list_new(name)(constr, dest, cmp)
#define list_is_empty(list) (list->head == NULL)
#define list_purge(list) list->purge(list)
#define list_free(list) list->free(list)
#define list_new_item(list) list->item_constructor ? list->item_constructor() : NULL
#define list_for_each(list, iter) \
	for (list_node(list) *__node = list->head; __node && (iter = __node->data, 1); __node = __node->next)

#define list_generator(T, name) \
	typedef T (*name##_item_constructor)(); \
	typedef void (*name##_item_destructor)(T item); \
	typedef int (*name##_comparator)(const void *v1, const void *v2); \
	\
	struct list_node_##name { \
		T data; \
		struct list_node_##name *next; \
	}; \
	\
	typedef struct name { \
		struct list_node_##name *head; \
		\
		void (*purge)(struct name *list); \
		void (*free)(struct name *list); \
		int (*push)(struct name *list, T item); \
		int (*pop)(struct name *list, T item); \
		T (*find)(struct name *list, T data); \
		int (*comparator)(const void *item1, const void *item2); \
		void (*item_destructor)(T item); \
		T (*item_constructor)(); \
	} name##_t; \
	\
	static T __list_find(name)(struct name *list, T what) \
	{ \
		list_node(list) *iter = list->head; \
		\
		if (!list->head || !list->comparator) \
			return NULL; \
		\
		while (iter) \
		{ \
			if (0 == list->comparator((const void *) iter->data, (const void *) what)) \
				return iter->data; \
			iter = iter->next; \
		} \
		\
		return NULL; \
	} \
	\
	static int __list_push(name)(struct name *list, T item) \
	{ \
		list_node(list) *temp = (list_node(list) *) calloc(1, sizeof(list_node(list))); \
		if (!temp) \
			return ENOMEM; \
		\
		temp->data = item; \
		\
		temp->next = list->head; \
		list->head = temp; \
		\
		return 0; \
	} \
	\
	static int __list_pop(name)(struct name *list, T val) \
	{ \
		list_node(list) *curr = NULL; \
		list_node(list) *prev = NULL; \
		\
		if (!list->comparator) \
			return EPERM; \
		\
		for (curr = list->head; curr; prev = curr, curr = curr->next) { \
			if (0 == list->comparator((const void *) curr->data, (const void *) val)) { \
				if (!prev) \
					list->head = curr->next; \
				else \
					prev->next = curr->next; \
				\
				if (list->item_destructor) \
					list->item_destructor(curr->data); \
				else \
					free(curr->data); \
				\
				free(curr); \
				\
				return 0; \
			} \
		} \
		\
		return ENODATA; \
	} \
	\
	static void __list_free(name)(struct name *list) \
	{ \
		list_node(list) *cursor = list->head; \
		\
		while (cursor) \
		{ \
			list_node(list) *tmp = cursor->next; \
			\
			if (list->item_destructor) \
				list->item_destructor(cursor->data); \
			else \
				free(cursor->data); \
			free(cursor); \
			\
			cursor = tmp; \
		} \
		\
		free(list); \
		list = NULL; \
	} \
	\
	static void __list_purge(name)(struct name *list) \
	{ \
		if (list->head) { \
			list_node(list) *cursor = list->head; \
			\
			while (cursor) \
			{ \
				list_node(list) *tmp = cursor->next; \
				if (list->item_destructor) \
					list->item_destructor(cursor->data); \
				else \
					free(cursor->data); \
				free(cursor); \
				\
				cursor = tmp; \
			} \
			list->head = NULL; \
		} \
	} \
	\
	static struct name *__list_new(name)(name##_item_constructor constructor, name##_item_destructor destructor, name##_comparator comparator) \
	{ \
		struct name *list = (struct name *) calloc(1, sizeof(struct name)); \
		if (!list) \
			return NULL; \
		\
		list->head = NULL; \
		\
		list->push = __list_push(name); \
		list->pop = __list_pop(name); \
		list->find = __list_find(name); \
		list->free = __list_free(name); \
		list->purge = __list_purge(name); \
		\
		list->comparator = comparator; \
		list->item_destructor = destructor; \
		list->item_constructor = constructor; \
		\
		return list; \
	}

#endif // __LIST_H__
