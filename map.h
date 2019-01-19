#ifndef __MAP_H__
#define __MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "list.h"

/*
 * map stores dynamically allocated objects which ones can be found by key
 */

#define __map_push(name)       __map_## name ##_push
#define __map_find(name)       __map_## name ##_find
#define __map_find_fmt(name)   __map_## name ##_find_fmt
#define __map_pop(name)        __map_## name ##_pop
#define __map_new(name)        __map_## name ##_new
#define __map_free(name)       __map_## name ##_free
#define __map_purge(name)      __map_## name ##_purge

#define __map_key_val(name) struct __map_key_val_##name
#define map_iter(map) list_data(map->list)
#define map_key(iter) (iter)->key
#define map_val(iter) (iter)->val

#define __STATIC_IF(cond, if_true, if_false, ...) \
	_Generic(&(int[(!!(cond)) + 1]) { 0 }, \
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
#define map_push(map, key, val) (map)->push(map, key, val)
#define map_pop(map, key) (map)->pop(map, key)
#define map_find(map, what) (map)->find(map, what)
#define map_find_fmt(map, ...) (map)->find_fmt(map, __VA_ARGS__)
#define map_is_empty(map) (list_is_empty((map)->list))
#define map_len(map) list_len(map->list)
#define map_purge(map) (map)->purge(map)
#define map_free(map) ({ if (map) (map)->free(&(map)); })
#define map_val_new(map, ...) (map)->item_constructor ? (map)->item_constructor(__VA_ARGS__) : NULL
#define map_val_free(map, val) (map)->item_destructor ? (map)->item_destructor(val) : free(val)
#define map_for_each(map, iter) list_for_each(map->list, iter)
#define map_for_each_safe(map, node, iter) list_for_each_safe(map->list, node, iter)
#define map_pop_safe(map, node) ({ \
	__STATIC_IF(__key_is_string(node_data(node)->key), __free_if_string, __do_nothing_map, node_data(node)->key); \
	map_val_free(map, node_data(node)->val); \
	list_pop_safe(map->list, node); \
	0; \
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
		return ERR_OK; \
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
		\
		if (NULL == map->comparator) \
			return NULL; \
		\
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
		if (NULL == val) \
			return ERR_INVALID_ARG; \
		else if (__map_find(name)(map, key)) \
			return ERR_DUPLICATED; \
		\
		if (NULL == (tmp = calloc(1, sizeof(__map_key_val(name))))) \
			return ERR_NO_MEM; \
		\
		if (__STATIC_IF(__key_is_string(key), __push_if_string, __push_if_not_string_##name, &tmp->key, key)) \
		{ \
			free(tmp); \
			return ERR_NO_MEM; \
		} \
		\
		tmp->val = val; \
		\
		if (list_push(map->list, tmp)) \
		{ \
			__STATIC_IF(__key_is_string(key), __free_if_string, __do_nothing_map, key); \
			free(tmp); \
			return ERR_NO_MEM; \
		} \
		\
		return ERR_OK; \
	} \
	\
	static int __map_pop(name)(struct name *map, T_key key) \
	{ \
		map_iter(map) *iter; \
		\
		if (NULL == map->comparator) \
			return ERR_NO_COMPARATOR; \
		\
		map_for_each_safe(map, node, iter) \
		{ \
			if (0 == map->comparator(map_key(iter), key))\
				return map_pop_safe(map, node); \
		} \
		\
		return ERR_NOT_FOUND; \
	} \
	\
	static void __map_purge(name)(struct name *map) \
	{ \
		map_iter(map) *iter = NULL; \
		\
		map_for_each_safe(map, node, iter) \
			map_pop_safe(map, node); \
		map->list->head = NULL; \
	} \
	\
	static void __map_free(name)(struct name **pmap) \
	{ \
		map_purge(*pmap); \
		\
		list_free((*pmap)->list); \
		free(*pmap); \
		*pmap = NULL; \
	} \
	\
	static struct name *__map_new(name)(void) \
	{ \
		struct name *map = NULL; \
		\
		if (NULL == (map = calloc(1, sizeof(struct name)))) \
			return NULL; \
		else if (NULL == (map->list = list_new(item_list_##name))) \
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
