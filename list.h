#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * linked list contains dynamically allocated objects
 */

#define __list_push(name)      __list_## name ##_push
#define __list_find(name)      __list_## name ##_find
#define __list_pop(name)       __list_## name ##_pop
#define __list_new(name)       __list_## name ##_new
#define __list_free(name)      __list_## name ##_free
#define __list_purge(name)      __list_## name ##_purge

#define list_node(list) __typeof(*list->head)
#define list_data(list) __typeof(*list->head->data)

static inline void __do_nothing_list() {}

#define list_push(list, item) list->push(list, item)
#define list_pop(list, item) list->pop(list, item)
#define list_pop_safely(list, iter /* stub */) ({ \
	if (!__prev_node) \
		list->head = __curr_node->next; \
	else \
		__prev_node->next = __curr_node->next; \
	\
	list_destroy_item(list, __curr_node->data); \
	free(__curr_node); \
})
#define list_find(list, what) list->find(list, what)
#define list_new(name) __list_new(name)()
#define list_is_empty(list) (list->head == NULL)
#define list_purge(list) list->purge(list)
#define list_free(list) ({ if (list) list->free(&list); })
#define list_new_val(list, ...) list->item_constructor ? list->item_constructor(__VA_ARGS__) : NULL
#define list_destroy_item(list, item) list->item_destructor ? list->item_destructor(item) : free(item)
#define list_for_each(list, iter) \
	for (list_node(list) *__node = list->head; __node && (iter = __node->data, 1); __node = __node->next)
#define list_for_each_safe(list, iter) \
	for (list_node(list) *__curr_node = list->head, *__prev_node = NULL; __curr_node && (iter = __curr_node->data, 1); __prev_node = __curr_node, __curr_node = __curr_node->next)

#define list_generator(T, name, __constructor, __destructor, __comparator) \
	struct list_node_##name { \
		T data; \
		struct list_node_##name *next; \
	}; \
	\
	typedef struct name { \
		struct list_node_##name *head; \
		\
		void (*purge)(struct name *list); \
		void (*free)(struct name **list); \
		int (*push)(struct name *list, T item); \
		int (*pop)(struct name *list, T item); \
		T (*find)(struct name *list, T data); \
		int (*comparator)(T item1, T item2); \
		void (*item_destructor)(T item); \
		T (*item_constructor)(); \
	} name##_t; \
	\
	static T __list_find(name)(struct name *list, T what) \
	{ \
		list_data(list) *iter; \
		\
		if (!list->head || !list->comparator) \
			return NULL; \
		\
		list_for_each(list, iter) \
		{ \
			if (0 == list->comparator(iter, what)) \
				return iter; \
		} \
		\
		return NULL; \
	} \
	\
	static int __list_push(name)(struct name *list, T item) \
	{ \
		list_node(list) *temp = NULL; \
		\
		if (NULL == (temp = calloc(1, sizeof(list_node(list))))) \
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
		list_data(list) *iter = NULL; \
		\
		if (!list->comparator) \
			return EPERM; \
		\
		list_for_each_safe(list, iter) \
		{ \
			if (0 == list->comparator(iter, val))\
			{ \
				list_pop_safely(list, iter); \
				return 0; \
			} \
		} \
		\
		return ENODATA; \
	} \
	\
	static void __list_purge(name)(struct name *list) \
	{ \
		list_data(list) *iter = NULL; \
		\
		list_for_each_safe(list, iter) \
			list_pop_safely(list, iter); \
		\
		list->head = NULL; \
	} \
	\
	static void __list_free(name)(struct name **plist) \
	{ \
		struct name *list = *plist; \
		\
		list_purge(list); \
		\
		free(list); \
		*plist = NULL; \
	} \
	\
	static struct name *__list_new(name)(void) \
	{ \
		struct name *list = NULL; \
		\
		if (NULL == (list = calloc(1, sizeof(struct name)))) \
			return NULL; \
		\
		list->push = __list_push(name); \
		list->pop = __list_pop(name); \
		list->find = __list_find(name); \
		list->free = __list_free(name); \
		list->purge = __list_purge(name); \
		\
		list->comparator = __comparator; \
		list->item_destructor = __destructor; \
		list->item_constructor = __constructor; \
		\
		return list; \
	}

#endif // __LIST_H__
