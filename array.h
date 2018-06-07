#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define __aind(name) __i_ ## name

#define __array_push(name)  __arr_## name ##_push
#define __array_pop(name)   __arr_## name ##_pop
#define __array_at(name)    __arr_## name ##_at
#define __array_find(name)  __arr_## name ##_find
#define __array_new(name)   __arr_## name ##_new

#define array_new(name) __array_new(name)()
#define array_len(arr) arr->len
#define array_set_comparator(arr, c) arr->comparator = c
#define array_is_empty(arr) arr->len == 0
#define array_at(arr, i) arr->at(arr, i)
#define array_at_val(arr, i) *arr->at(arr, i) // !!! be careful
#define array_pop(arr, pos) arr->pop(arr, pos)
#define array_push(arr, item) arr->push(arr, item)
#define array_push_val(arr, val) ({ \
		__typeof__(val) __item = val; \
		arr->push(arr, &__item); \
	})
#define array_push_array(arr, from, len) \
	int __aind(from) = 0; \
	for ( ; __aind(from) < len; __aind(from)++) \
		array_push(arr, &from[__aind(from)]);


#define array_find(arr, what) arr->find(arr, what)
#define array_find_val(arr, val) ({ \
		__typeof__(val) __item = (val); \
		arr->find(arr, &__item); \
	})

#define array_free(arr) \
	if (arr) { \
		free(arr->data); \
		free(arr); \
	}

#define array_for_each(iter, arr) \
	__typeof(arr->data) iter; \
	int __aind(iter) = 0; \
	for ( ; __aind(iter) < array_len(arr) && (iter = array_at(arr, __aind(iter)), 1); ++__aind(iter))

#define array_for_each_val(iter, arr) \
	__typeof(array_at_val(arr, 0)) iter; \
	int __aind(iter) = 0; \
	for ( ; __aind(iter) < array_len(arr) && (iter = array_at_val(arr, __aind(iter)), 1); ++__aind(iter))

#define __array_new_gen(T, name) \
	int __array_push(name)(struct name *arr, T *item) \
	{ \
		if (!arr) \
			return EACCES; \
		if (arr->capacity == arr->len) { \
			arr->capacity *= 2; \
			arr->data = (T *) realloc(arr->data, arr->capacity * sizeof(T)); \
			if (!arr->data) \
				return ENOMEM; \
		} \
		\
		memcpy((void *) &arr->data[arr->len++], (void *) item, sizeof(T)); \
		\
		return 0; \
	} \
	\
	int __array_pop(name)(struct name *arr, int pos) \
	{ \
		if (!arr) \
			return EACCES; \
		if (array_is_empty(arr)) \
			return ENODATA; \
		if ((pos >= array_len(arr)) || (pos < -array_len(arr))) \
			return EPERM; \
		/* case for len == 1, or for the last elem */ \
		else if ((pos == array_len(arr) - 1) || (pos == -1)) \
			arr->len -= 1; \
		else if ((pos >= 0) && (pos < array_len(arr) - 1)) { \
			int i; \
			for (i = pos; i < array_len(arr) - 1; ++i) \
				memcpy((void *) array_at(arr, i), (void *) array_at(arr, i+1), sizeof(T)); \
			arr->len -= 1; \
		} else if ((pos < 0) && (pos >= -array_len(arr))) { \
			int i = array_len(arr) + pos; \
			for (; i < array_len(arr) - 1; ++i) \
				memcpy((void *) array_at(arr, i), (void *) array_at(arr, i+1), sizeof(T)); \
			arr->len -= 1; \
		} \
		\
		return 0; \
	} \
	\
	T *__array_at(name)(struct name *arr, int pos) \
	{ \
		if (!arr) \
			return NULL; \
		if (pos >= 0) \
			return pos < array_len(arr) ? &arr->data[pos] : NULL; \
		else \
			return pos >= -array_len(arr) ? &arr->data[array_len(arr)+pos] : NULL; \
	} \
	\
	int __array_find(name)(struct name *arr, T *what) \
	{ \
		if (!arr) \
			return -EACCES; \
		if (!arr->comparator) \
			return -EPERM; \
		\
		array_for_each(iter, arr) { \
			if (arr->comparator(iter, what)) \
				return __aind(iter); \
		} \
		\
		return -ENOENT; \
	} \
	\
	struct name *__array_new(name)() \
	{ \
		struct name *arr = (struct name *) calloc(1, sizeof(struct name)); \
		if (!arr) \
			return NULL; \
		arr->capacity = 10; \
		arr->data = (T *) calloc(arr->capacity, sizeof(T)); \
		if (!arr->data) { \
			free(arr); \
			return NULL; \
		} \
		arr->len = 0; \
		\
		arr->push = __array_push(name); \
		arr->pop  = __array_pop(name); \
		arr->at   = __array_at(name); \
		arr->find = __array_find(name); \
		arr->comparator = NULL; \
		\
		return arr; \
	}

#define array_generator(T, name) \
	typedef struct name { \
		T *data; \
		int len; \
		int capacity; \
		\
		int (*push)(struct name *arr, T *item); \
		int (*pop)(struct name *arr, int pos); \
		T *(*at)(struct name *arr, int pos); \
		int (*find)(struct name *arr, T *item); \
		int (*comparator)(T *item1, T *item2); \
	} name##_t; \
	\
	__array_new_gen(T, name)

#endif // __ARRAY_H__
