#ifndef __MAP_H__
#define __MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "list.h"

/*
 * map contains dynamically allocated objects which ones can be found by key
 */

#define __map_push(name)       __map_## name ##_push
#define __map_find(name)       __map_## name ##_find
#define __map_find_fmt(name)   __map_## name ##_find_fmt
#define __map_pop(name)        __map_## name ##_pop
#define __map_new(name)        __map_## name ##_new
#define __map_free(name)       __map_## name ##_free
#define __map_purge(name)      __map_## name ##_purge

#define __map_key_val(name) struct __map_key_val_##name
#define map_iter(map) __typeof(*map->list->head->data)
#define map_key(iter) iter->key
#define map_val(iter) iter->val

#define __STATIC_IF(cond, if_true, if_false, ...) _Generic(&(int[(!!(cond)) + 1]) { 0 }, \
    int(*)[2]: if_true, \
    int(*)[1]: if_false) \
  (__VA_ARGS__)

#define __key_is_string(key) __builtin_types_compatible_p(__typeof(key), char *)
static inline void __do_nothing_map() {}
static inline void *__always_null_map() { return NULL; }
static inline int __push_if_string(char **to, char *key)
{
	if (NULL == (*to = strdup(key)))
		return ENOMEM;
	return 0;
}
static inline void __free_if_string(char *key) { free(key); }

#define map_new(name) __map_new(name)()
#define map_push(map, key, val) map->push(map, key, val)
#define map_pop(map, key) map->pop(map, key)
#define map_pop_safely(map, iter /* stub */) ({ \
	if (!__prev_node) \
		map->list->head = __curr_node->next; \
	else \
		__prev_node->next = __curr_node->next; \
	\
	map_destroy_item(map, __curr_node->data->val); \
	__STATIC_IF(__key_is_string(__curr_node->data->key), __free_if_string, __do_nothing_map, __curr_node->data->key); \
	list_destroy_item(map->list, __curr_node->data); \
	free(__curr_node); \
})
#define map_find(map, what) map->find(map, what)
#define map_find_fmt(map, ...) map->find_fmt(map, __VA_ARGS__)
#define map_is_empty(map) (list_is_empty(map->list))
#define map_set_comparator(map, c) map->comparator = c
#define map_purge(map) map->purge(map)
#define map_free(map) ({ if (map) map->free(&map); })
#define map_new_val(map) map->item_constructor ? map->item_constructor() : NULL
#define map_destroy_item(map, val) map->item_destructor ? map->item_destructor(val) : free(val)
#define map_for_each(map, iter) \
	for (list_node(map->list) *__node = map->list->head; __node && (iter = __node->data, 1); __node = __node->next)
#define map_for_each_safe(map, iter) \
	for (list_node(map->list) *__curr_node = map->list->head, *__prev_node = NULL; __curr_node && (iter = __curr_node->data, 1); __prev_node = __curr_node, __curr_node = __curr_node->next)

#define map_len(map) ({ \
	size_t len = 0; \
	map_iter(map) *iter; \
	map_for_each(map, iter) \
		++len; \
	len; \
})

#define map_generator(T_key, T_val, name, __constructor, __destructor, __comparator) \
	\
	__map_key_val(name) { \
		T_key key; \
		T_val val; \
	}; \
    \
	static int __push_if_not_string_##name(T_key *to, T_key key) \
	{ \
		memcpy(to, &key, sizeof(T_key)); \
		return 0; \
	} \
	\
	list_generator(__map_key_val(name) *, item_list_##name, NULL, NULL, NULL) \
	\
	typedef struct name { \
		struct item_list_##name *list; \
		\
		void (*free)(struct name **map); \
		void (*purge)(struct name *map); \
		int (*push)(struct name *map, T_key key, T_val val); \
		int (*pop)(struct name *map, T_key key); \
		T_val (*find)(struct name *map, T_key key); \
		T_val (*find_fmt)(struct name *map, char *fmt, ...); \
		int (*comparator)(T_key key1, T_key key2); \
		void (*item_destructor)(T_val val); \
		T_val (*item_constructor)(); \
	} name##_t; \
	\
	static T_val __map_find(name)(struct name *map, T_key key) \
	{ \
		list_data(map->list) *iter = NULL; \
		if (!map->comparator) \
			return NULL; \
		list_for_each(map->list, iter) \
		{ \
			if (0 == map->comparator(iter->key, key)) \
				return iter->val; \
		} \
		\
		return NULL; \
	} \
    \
	static T_val __map_find_fmt(name)(struct name *map, char *fmt, ...) \
	{ \
		va_list args; \
		char key[1024] = { 0 }; \
		int ret = 0; \
		\
		va_start(args, fmt); \
			ret = vsnprintf(key, sizeof(key), fmt, args); \
		va_end(args); \
		\
		return (ret < 0 || ret >= sizeof(key)) ? NULL : __STATIC_IF(__key_is_string(T_key), __map_find(name), __always_null_map, map, key); \
	} \
	\
	static void __set_find_fmt_##name(struct name *map) \
	{ \
		map->find_fmt = __map_find_fmt(name); \
	} \
	\
	static int __map_push(name)(struct name *map, T_key key, T_val val) \
	{ \
		list_data(map->list) *tmp = NULL; \
		\
		if (__map_find(name)(map, key)) \
			return EEXIST; \
		\
		if (!val) \
			return ENODATA; \
		\
		if (NULL == (tmp = calloc(1, sizeof(__map_key_val(name))))) \
			return ENOMEM; \
		\
		if (__STATIC_IF(__key_is_string(key), __push_if_string, __push_if_not_string_##name, &tmp->key, key)) \
		{ \
			free(tmp); \
			return ENOMEM; \
		} \
		\
		tmp->val = val; \
		\
		if (list_push(map->list, tmp)) \
		{ \
			__STATIC_IF(__key_is_string(key), __free_if_string, __do_nothing_map, key); \
			free(tmp); \
			return ENOMEM; \
		} \
		\
		return 0; \
	} \
	\
	static int __map_pop(name)(struct name *map, T_key key) \
	{ \
		int ret = 0; \
		map_iter(map) *iter; \
		\
		if (!map->comparator) \
			return EPERM; \
		\
		map_for_each_safe(map, iter) \
		{ \
			if (0 == map->comparator(map_key(iter), key))\
			{ \
				map_pop_safely(map, iter); \
				return 0; \
			} \
		} \
		\
		return ENODATA; \
	} \
	\
	static void __map_purge(name)(struct name *map) \
	{ \
		map_iter(map) *iter; \
		\
		map_for_each_safe(map, iter) \
			map_pop_safely(map, iter); \
		map->list->head = NULL; \
	} \
	\
	static void __map_free(name)(struct name **pmap) \
	{ \
		struct name *map = *pmap; \
		\
		map_purge(map); \
		\
		list_free(map->list); \
		free(map); \
		*pmap = NULL; \
	} \
	\
	static struct name *__map_new(name)(void) \
	{ \
		struct name *map = NULL; \
		\
		if (NULL == (map = calloc(1, sizeof(struct name)))) \
			return NULL; \
		\
		if (NULL == (map->list = list_new(item_list_##name))) \
		{ \
			free(map); \
			return NULL; \
		} \
		\
		map->push = __map_push(name); \
		map->pop = __map_pop(name); \
		map->find = __map_find(name); \
		__STATIC_IF(__key_is_string(T_key), __set_find_fmt_##name, __do_nothing_map, map); \
		map->free = __map_free(name); \
		map->purge = __map_purge(name); \
		\
		map->comparator = __comparator; \
		map->item_destructor = __destructor; \
		map->item_constructor = __constructor; \
		\
		return map; \
	}

#endif // __MAP_H__
