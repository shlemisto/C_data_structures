#ifndef __MAP_H__
#define __MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list.h"

/*
 * map to contain dynamically allocated objects which ones can be found by key
 */

#define __map_push(name)      __map_## name ##_push
#define __map_find(name)      __map_## name ##_find
#define __map_pop(name)       __map_## name ##_pop
#define __map_pop_head(name)  __map_## name ##_pop_head
#define __map_pop_tail(name)  __map_## name ##_pop_tail
#define __map_new(name)       __map_## name ##_new
#define __map_free(name)      __map_## name ##_free
#define __map_purge(name)     __map_## name ##_purge

#define map_key_val(name) struct map_key_val_##name
#define map_key(iter) iter->key
#define map_val(iter) iter->val
#define map_node(map) __typeof(*map->list->head->data)

#define map_push(map, key, val) map->push(map, key, val)
#define map_pop(map, key) map->pop(map, key)
#define map_find(map, what) map->find(map, what)
#define map_is_empty(map) (list_is_empty(map->list))
#define map_new(name, constr, dest, cmp) __map_new(name)(constr, dest, cmp)
#define map_purge(map) map->purge(map)
#define map_free(map) map->free(map)
#define map_new_val(map) map->item_constructor ? map->item_constructor() : NULL
#define map_for_each(map, iter) \
	for (list_node(map->list) *__node = map->list->head; __node && (iter = __node->data, 1); __node = __node->next)

#define map_generator(T_key, T_val, name) \
	\
	map_key_val(name) { \
		T_key key; \
		T_val val; \
	}; \
	\
	typedef T_val (*name##_item_constructor)(); \
	typedef void (*name##_item_destructor)(T_val val); \
	typedef int (*name##_comparator)(const void *v1, const void *v2); \
	\
	list_generator(map_key_val(name) *, item_list_##name) \
	\
	typedef struct name { \
		struct item_list_##name *list; \
		\
		void (*free)(struct name *map); \
		void (*purge)(struct name *map); \
		int (*push)(struct name *map, T_key key, T_val val); \
		int (*pop)(struct name *map, T_key key); \
		T_val (*find)(struct name *map, T_key key); \
		int (*comparator)(const void *item1, const void *item2); \
		void (*item_destructor)(T_val val); \
		T_val (*item_constructor)(); \
	} name##_t; \
	\
	static T_val __map_find(name)(struct name *map, T_key key) \
	{ \
		list_data(map->list) *iter = NULL; \
		if (!map->comparator || !map->list) \
			return NULL; \
		list_for_each(map->list, iter) { \
			if (0 == map->comparator((const void *) &iter->key, (const void *) &key)) \
				return iter->val; \
		} \
		\
		return NULL; \
	} \
	\
	static int __map_push(name)(struct name *map, T_key key, T_val val) \
	{ \
		list_data(map->list) *temp = NULL; \
		\
		if (__map_find(name)(map, key)) \
			return EEXIST; \
		\
		temp = (map_key_val(name) *) calloc(1, sizeof(map_key_val(name))); \
		if (!temp) \
			return ENOMEM; \
		\
		memcpy(&temp->key, &key, sizeof(key)); \
		temp->val = val; \
		\
		if (list_push(map->list, temp)) { \
			free(temp); \
			return ENOMEM; \
		} \
		\
		return 0; \
	} \
	\
	static int __map_pop(name)(struct name *map, T_key key) \
	{ \
		list_node(map->list) *curr = NULL; \
		list_node(map->list) *prev = NULL; \
		\
		if (!map->comparator) \
			return EPERM; \
		\
		\
		for (curr = map->list->head; curr; prev = curr, curr = curr->next) { \
			if (0 == map->comparator((const void *) &curr->data->key, (const void *) &key)) { \
				if (!prev) \
					map->list->head = curr->next; \
				else \
					prev->next = curr->next; \
				\
				if (map->item_destructor) \
					map->item_destructor(curr->data->val); \
				else \
					free(curr->data->val); \
				\
				free(curr->data); \
				free(curr); \
				\
				return 0; \
			} \
		} \
		\
		return ENODATA; \
	} \
	\
	static void __map_free(name)(struct name *map) \
	{ \
		if(map) { \
			list_node(map->list) *cursor = map->list->head; \
			\
			while (cursor) \
			{ \
				list_node(map->list) *tmp = cursor->next; \
				\
				if (map->item_destructor) \
					map->item_destructor(cursor->data->val); \
				else \
					free(cursor->data->val); \
				\
				free(cursor->data); \
				free(cursor); \
				\
				cursor = tmp; \
			} \
			\
			free(map->list); \
			free(map); \
			\
			map = NULL; \
		} \
	} \
	\
	static void __map_purge(name)(struct name *map) \
	{ \
		list_node(map->list) *cursor = map->list->head; \
		\
		while (cursor) \
		{ \
			list_node(map->list) *tmp = cursor->next; \
			\
			if (map->item_destructor) \
				map->item_destructor(cursor->data->val); \
			else \
				free(cursor->data->val); \
			\
			free(cursor->data); \
			free(cursor); \
			\
			cursor = tmp; \
		} \
		\
		map->list->head = NULL; \
	} \
	\
	static struct name *__map_new(name)(name##_item_constructor constructor, name##_item_destructor destructor, name##_comparator comparator) \
	{ \
		struct name *map = (struct name *) calloc(1, sizeof(struct name)); \
		if (!map) \
			return NULL; \
		\
		map->list = list_new(item_list_##name, NULL, NULL, NULL); \
		if (!map->list) { \
			free(map); \
			return NULL; \
		} \
		\
		map->push = __map_push(name); \
		map->pop = __map_pop(name); \
		map->find = __map_find(name); \
		map->free = __map_free(name); \
		map->purge = __map_purge(name); \
		\
		map->comparator = comparator; \
		map->item_destructor = destructor; \
		map->item_constructor = constructor; \
		\
		return map; \
	}

#endif // __MAP_H__
