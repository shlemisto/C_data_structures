#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "defs.h"

/*
 * double linked list stores dynamically allocated objects
 */

#define __list_push(name)        __list_## name ##_push
#define __list_pop(name)         __list_## name ##_pop
#define __list_enqueue(name)     __list_## name ##_enqueue
#define __list_pop_safe(name)    __list_## name ##_pop_safe
#define __list_pop_by_val(name)  __list_## name ##_pop_by_val
#define __list_find(name)        __list_## name ##_find
#define __list_peek(name)        __list_## name ##_peek
#define __list_new(name)         __list_## name ##_new
#define __list_free(name)        __list_## name ##_free
#define __list_purge(name)       __list_## name ##_purge

#define list_node_t(list) __typeof(*(list)->head)
#define list_data_t(list) __typeof(*(list)->head->__priv_data)

#define list_head(list) (list)->head
#define list_tail(list) (list)->tail
#define list_node_next(node) (node)->next
#define list_node_prev(node) (node)->prev
#define list_node_data(node) (node)->__priv_data

static inline void __do_nothing_list() {}

// stack
#define list_push(list, item) (list)->push(list, item)
#define list_pop(list) (list)->pop(list, 1, 0)

// queue
#define list_enqueue(list, item) (list)->enqueue(list, item)
#define list_dequeue(list) (list)->pop(list, 0, 0)

#define list_find(list, what) (list)->find(list, what)
#define list_len(list) ({ \
	size_t len = 0; \
	for (list_node_t(list) *node = list_head(list), *safe_node = NULL; node && (safe_node = list_node_next(node), 1); node = safe_node) \
		++len; \
	len; \
})
#define list_peek_head(list) (list)->peek(list, 1, 0)
#define list_peek_tail(list) (list)->peek(list, 0, 0)
#define list_steal_head(list) (list)->peek(list, 1, 1)
#define list_steal_tail(list) (list)->peek(list, 0, 1)
#define list_pop_by_val(list, val) (list)->pop_by_val(list, val)
#define list_new(name) __list_new(name)()
#define list_is_empty(list) (list_head(list) == NULL)
#define list_purge(list) (list)->purge(list)
#define list_free(list) ({ if (list) (list)->free(&(list)); })
#define list_val_new(list, ...) (list)->item_constructor ? (list)->item_constructor(__VA_ARGS__) : NULL
#define list_val_free(list, item) (list)->item_destructor ? (list)->item_destructor(item) : free(item)
#define list_for_each(list, user_data) \
	for (list_node_t(list) *node = list_head(list); node && (user_data = list_node_data(node), 1); node = list_node_next(node))
#define list_for_each_inverse(list, user_data) \
	for (list_node_t(list) *node = list_tail(list); node && (user_data = list_node_data(node), 1); node = list_node_prev(node))

#define list_for_each_safe(list, node, user_data) \
	for (list_node_t(list) *node = list_head(list), *safe_node = NULL; node && (user_data = list_node_data(node), 1) && (safe_node = list_node_next(node), 1); node = safe_node)
#define list_pop_safe(list, node) (list)->pop_safe(list, node)

#define list_generator(T, name, __constructor, __destructor, __comparator) \
	struct list_node_##name { \
		T __priv_data; \
		struct list_node_##name *next, *prev; \
	}; \
	\
	typedef struct name { \
		struct list_node_##name *head, *tail; \
		\
		void (*purge)(struct name *list); \
		void (*free)(struct name **list); \
		int (*pop_safe)(struct name *list, struct list_node_##name *node); \
		int (*pop_by_val)(struct name *list, T val); \
		int (*push)(struct name *list, T item); \
		int (*pop)(struct name *list, int pop_head, int steal); \
		int (*enqueue)(struct name *list, T item); \
		T (*find)(struct name *list, T data); \
		T (*peek)(struct name *list, int peek_head, int steal); \
		int (*comparator)(T item1, T item2); \
		void (*item_destructor)(T item); \
		T (*item_constructor)(); \
	} name##_t; \
	\
	static T __list_find(name)(struct name *list, T what) \
	{ \
		T iter = NULL; \
		\
		if (list_is_empty(list) || NULL == list->comparator) \
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
	static T __list_peek(name)(struct name *list, int peek_head, int steal) \
	{ \
		T data = NULL; \
		\
		if (list_is_empty(list)) \
			return NULL; \
		\
		data = peek_head ? list_node_data(list_head(list)) : list_node_data(list_tail(list)); \
		\
		if (steal) \
			list->pop(list, peek_head, steal); \
		\
		return data; \
	} \
	\
	static int __list_enqueue(name)(struct name *list, T item) \
	{ \
		list_node_t(list) *temp = NULL; \
		\
		if (NULL == item) \
			return ERR_INVALID_ARG; \
		\
		if (NULL == (temp = calloc(1, sizeof(list_node_t(list))))) \
			return ERR_NO_MEM; \
		\
		list_node_data(temp) = item; \
		\
		if (list_is_empty(list)) \
			list_head(list) = list_tail(list) = temp; \
		else \
		{ \
			list_node_next(list_tail(list)) = temp; \
			list_node_prev(temp) = list_tail(list); \
			list_tail(list) = temp; \
		} \
		\
		return ERR_OK; \
	} \
	\
	static int __list_push(name)(struct name *list, T item) \
	{ \
		list_node_t(list) *temp = NULL; \
		\
		if (NULL == item) \
			return ERR_INVALID_ARG; \
		\
		if (NULL == (temp = calloc(1, sizeof(list_node_t(list))))) \
			return ERR_NO_MEM; \
		\
		list_node_data(temp) = item; \
		\
		if (list_is_empty(list)) \
			list_head(list) = list_tail(list) = temp; \
		else \
		{ \
			list_node_next(temp) = list_head(list); \
			list_node_prev(list_head(list)) = temp; \
			list_head(list) = temp; \
		} \
		\
		return ERR_OK; \
	} \
	\
	static int __list_pop(name)(struct name *list, int pop_head, int steal) \
	{ \
		list_node_t(list) *node = NULL; \
		\
		if (list_is_empty(list)) \
			return ERR_EMPTY; \
		\
		node = pop_head ? list_head(list) : list_tail(list); \
		\
		if (list_head(list) == list_tail(list)) \
			list_head(list) = list_tail(list) = NULL; \
		else if (pop_head) \
		{ \
			list_head(list) = list_node_next(list_head(list)); \
			if (list_head(list)) \
				list_node_prev(list_head(list)) = NULL; \
		} \
		else \
		{ \
			list_tail(list) = list_node_prev(list_tail(list)); \
			if (list_tail(list)) \
				list_node_next(list_tail(list)) = NULL; \
		} \
		\
		if (0 == steal) \
			list_val_free(list, list_node_data(node)); \
		free(node); \
		\
		return ERR_OK; \
	} \
	\
	static int __list_pop_safe(name)(struct name *list, list_node_t(list) *node) \
	{ \
		int ret = 0; \
		\
		if (list_is_empty(list)) \
			ret = ERR_EMPTY; \
		else if (node == list_head(list)) \
			ret = list_pop(list); \
		else if (node == list_tail(list)) \
			ret = list_dequeue(list); \
		else \
		{ \
			list_node_t(list) *next = list_node_next(node); \
			list_node_t(list) *prev = list_node_prev(node); \
			\
			list_node_prev(next) = prev; \
			list_node_next(prev) = next; \
			\
			list_val_free(list, list_node_data(node)); \
			free(node); \
		} \
		\
		return ret; \
	} \
	\
	static int __list_pop_by_val(name)(struct name *list, T what) \
	{ \
		T iter = NULL; \
		\
		if (list_is_empty(list)) \
			return ERR_EMPTY; \
		if (NULL == list->comparator) \
			return ERR_NO_COMPARATOR; \
		\
		list_for_each_safe(list, node, iter) \
		{ \
			if (0 == list->comparator(iter, what)) \
				return list_pop_safe(list, node); \
		} \
		\
		return ERR_NOT_FOUND; \
	} \
	\
	static void __list_purge(name)(struct name *list) \
	{ \
		for (list_node_t(list) *node = list_head(list), *safe_node = NULL; node && (safe_node = list_node_next(node), 1); node = safe_node) \
			(void) list_pop_safe(list, node); \
		\
		list_head(list) = NULL; \
	} \
	\
	static void __list_free(name)(struct name **plist) \
	{ \
		list_purge(*plist); \
		\
		free(*plist); \
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
		list->enqueue = __list_enqueue(name); \
		list->pop = __list_pop(name); \
		list->find = __list_find(name); \
		list->pop_safe = __list_pop_safe(name); \
		list->pop_by_val = __list_pop_by_val(name); \
		list->peek = __list_peek(name); \
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
