#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

// get index from iterator
#define __aind(name) __i_ ## name

#define __array_push_array(name)  __arr_## name ##_push_array
#define __array_push(name)	 __arr_## name ##_push
#define __array_pop(name)	 __arr_## name ##_pop
#define __array_pop_by_ind(name) __arr_## name ##_pop_by_ind
#define __array_at(name)	 __arr_## name ##_at
#define __array_find(name)	 __arr_## name ##_find
#define __array_new(name)	 __arr_## name ##_new

#define array_new(name, comparator) __array_new(name)(comparator)
#define array_len(arr) arr->len
#define array_data(arr) arr->data
#define array_is_empty(arr) arr->len == 0
#define array_set_comparator(arr, c) arr->comparator = c
#define array_at(arr, i) arr->at(arr, i)
#define array_at_val(arr, i) *arr->at(arr, i) // !!! be careful
#define array_pop(arr, pos) arr->pop(arr, pos)
#define array_pop_by_ind(arr, pos) arr->pop_by_ind(arr, pos)
#define array_push(arr, item) arr->push(arr, item)
#define array_push_array(arr, from, len) arr->push_array(arr, from, len)
#define array_find_from(arr, what, pos) arr->find(arr, what, pos)
#define array_find(arr, what) array_find_from(arr, what, 0)
#define array_find_val(arr, val) array_find_val_from(arr, val, 0)
#define array_purge(arr) \
	if (arr) { \
		arr->len = 0; \
		arr->capacity = 10; \
	}
#define array_free(arr) \
	if (arr) { \
		free(arr->data); \
		free(arr); \
		arr = NULL; \
	}

// note: pos >= 0
#define __array_for_each(iter, arr, pos) \
	for (int __aind(iter) = (pos); ((pos) >= 0) && (__aind(iter) < array_len(arr)) && (iter = array_at(arr, __aind(iter)), 1); ++__aind(iter))
#define array_for_each(iter, arr) \
	__array_for_each(iter, arr, 0)
#define array_for_each_val(iter, arr) \
	for (int __aind(iter) = 0 ; __aind(iter) < array_len(arr) && (iter = array_at_val(arr, __aind(iter)), 1); ++__aind(iter))

#define __array_new_gen(T, name) \
	typedef T (*name##_item_constructor_p)(); \
	typedef void (*name##_item_destructor_p)(T item); \
	typedef int (*name##_comparator)(const void *v1, const void *v2); \
	\
	int __array_push(name)(struct name *arr, T *item) \
	{ \
		if (!item) \
			return EPERM; \
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
	int __array_push_array(name)(struct name *arr, T *from, int len) \
	{ \
		int ret = 0; \
		for (int i = 0 ; i < len; ++i) { \
			if ((ret = __array_push(name)(arr, &from[i]))) \
			    break; \
		} \
		return ret; \
	} \
	\
	int __array_pop_by_ind(name)(struct name *arr, int pos) \
	{ \
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
				memcpy((void *) &arr->data[i], (void *) &arr->data[i+1], sizeof(T)); \
			arr->len -= 1; \
		} else if ((pos < 0) && (pos >= -array_len(arr))) { \
			int i = array_len(arr) + pos; \
			for (; i < array_len(arr) - 1; ++i) \
				memcpy((void *) &arr->data[i], (void *) &arr->data[i+1], sizeof(T)); \
			arr->len -= 1; \
		} \
		\
		return 0; \
	} \
	\
	int __array_pop(name)(struct name *arr, T *addr) \
	{ \
		int ret = ENODATA; \
		__typeof(arr->data) iter = NULL; \
		\
		if (!addr) \
			return ret; \
		array_for_each(iter, arr) { \
			if (iter == addr) { \
				ret = __array_pop_by_ind(name)(arr, __aind(iter)); \
				break; \
			} \
		} \
		\
		return ret; \
	} \
	\
	T *__array_at(name)(struct name *arr, int pos) \
	{ \
		if (pos >= 0) \
			return pos < array_len(arr) ? &arr->data[pos] : NULL; \
		else \
			return pos >= -array_len(arr) ? &arr->data[array_len(arr)+pos] : NULL; \
	} \
	\
	T *__array_find(name)(struct name *arr, T *what, int pos) \
	{ \
		__typeof(arr->data) iter = NULL; \
		\
		if (!arr->comparator || !what) \
			return NULL; \
		\
		__array_for_each(iter, arr, pos) { \
			if (0 == arr->comparator((const void *) iter, (const void *) what)) \
				return iter; \
		} \
		\
		return NULL; \
	} \
	\
	struct name *__array_new(name)(name##_comparator comparator) \
	{ \
		struct name *arr = (struct name *) calloc(1, sizeof(struct name)); \
		if (!arr) \
			return NULL; \
		arr->len = 0; \
		arr->capacity = 10; \
		arr->data = (T *) calloc(arr->capacity, sizeof(T)); \
		if (!arr->data) { \
			free(arr); \
			return NULL; \
		} \
		\
		arr->push_array = __array_push_array(name); \
		arr->push = __array_push(name); \
		arr->pop = __array_pop(name); \
		arr->at = __array_at(name); \
		arr->find = __array_find(name); \
		arr->comparator = comparator; \
		arr->pop_by_ind = __array_pop_by_ind(name); \
		\
		return arr; \
	}

/* --> specialization for array of pointers */
#define __parray_push(name)	  __parr_## name ##_push
#define __parray_at(name)	  __parr_## name ##_at
#define __parray_find(name)	  __parr_## name ##_find
#define __parray_new(name)	  __parr_## name ##_new
#define __parray_pop(name)	  __parr_## name ##_pop
#define __parray_pop_by_ind(name) __parr_## name ##_pop_by_ind

#define parray_new(name, constructor, destructor, comparator) __parray_new(name)(constructor, destructor, comparator)
#define parray_len(arr) arr->len
#define parray_data(arr) arr->data
#define parray_is_empty(arr) (arr->len == 0)
#define parray_set_item_destructor(arr, d) arr->item_destructor_p = d
#define parray_set_comparator(arr, c) arr->comparator = c
#define parray_push(arr, item) arr->push_p(arr, item)
#define parray_push_array(arr, from, len) arr->push_array(arr, from, len)
#define parray_pop_by_ind(arr, pos) arr->pop_by_ind(arr, pos)
#define parray_pop(arr, addr) arr->pop_p(arr, addr)
#define parray_at(arr, i) arr->at_p(arr, i)
#define __parray_for_each(iter, arr, pos) \
	for (int __aind(iter) = (pos); ((pos) >= 0) && (__aind(iter) < array_len(arr)) && (iter = parray_at(arr, __aind(iter)), 1); ++__aind(iter))
#define parray_for_each(iter, arr) __parray_for_each(iter, arr, 0)
#define parray_find_from(arr, what, pos) arr->find_p(arr, what, pos)
#define parray_find(arr, what) parray_find_from(arr, what, 0)
#define parray_item_alloc(arr) arr->item_constructor_p ? arr->item_constructor_p() : NULL

#define parray_purge(arr) \
	if (arr) { \
		if (arr->item_destructor_p) { \
			int i; \
			int len = array_len(arr); \
			for (i = 0 ; i < len; ++i) \
				arr->item_destructor_p(parray_at(arr, i)); \
		} \
		arr->len = 0; \
		arr->capacity = 10; \
	}
#define parray_free(arr) \
	if (arr) { \
		if (arr->item_destructor_p) { \
			int i; \
			int len = array_len(arr); \
			for (i = 0 ; i < len; ++i) \
				arr->item_destructor_p(parray_at(arr, i)); \
		} \
		\
		free(arr->data); \
		free(arr); \
		arr = NULL; \
	}

#define parray_generator(T, name) \
	array_generator(T, name) \
	\
	int __parray_push(name)(struct name *arr, T item) \
	{ \
		if (item) \
			return __array_push(name)(arr, &item); \
		return EPERM; \
	} \
	\
	T __parray_at(name)(struct name *arr, int pos) \
	{ \
		T *item = __array_at(name)(arr, pos); \
		if (item) \
			return *__array_at(name)(arr, pos); \
		return NULL; \
	} \
	\
	int __parray_pop_by_ind(name)(struct name *arr, int pos) \
	{ \
		int ret = ENODATA; \
		T addr = __parray_at(name)(arr, pos); \
		if (addr) { \
			ret = __array_pop_by_ind(name)(arr, pos); \
			if (arr->item_destructor_p) \
				arr->item_destructor_p(addr); \
		} \
		\
		return ret; \
	} \
	\
	int __parray_pop(name)(struct name *arr, T addr) \
	{ \
		int ret = ENODATA; \
		__typeof(arr->data[0]) iter = NULL; \
		\
		if (!addr) \
			return ret; \
		parray_for_each(iter, arr) { \
			if (iter == addr) { \
				ret = __parray_pop_by_ind(name)(arr, __aind(iter)); \
				break; \
			} \
		} \
		\
		return ret; \
	} \
	\
	T __parray_find(name)(struct name *arr, T what, int pos) \
	{ \
		__typeof(arr->data[0]) iter = NULL; \
		\
		if (!arr->comparator || !what) \
			return NULL; \
		\
		__parray_for_each(iter, arr, pos) { \
			if (0 == arr->comparator((const void *) &iter, (const void *) &what)) \
				return iter; \
		} \
		\
		return NULL; \
	} \
	\
	struct name *__parray_new(name)(name##_item_constructor_p constructor, name##_item_destructor_p destructor, name##_comparator comparator) \
	{ \
		struct name *arr = (struct name *) calloc(1, sizeof(struct name)); \
		if (!arr) \
			return NULL; \
		arr->len = 0; \
		arr->capacity = 10; \
		arr->data = (T *) calloc(arr->capacity, sizeof(T)); \
		if (!arr->data) { \
			free(arr); \
			return NULL; \
		} \
		\
		arr->push_array = __array_push_array(name); \
		arr->push_p = __parray_push(name); \
		arr->pop_p = __parray_pop(name); \
		arr->pop_by_ind = __parray_pop_by_ind(name); \
		arr->at_p = __parray_at(name); \
		arr->comparator = comparator; \
		arr->find_p = __parray_find(name); \
		arr->item_destructor_p = destructor; \
		arr->item_constructor_p = constructor; \
		\
		return arr; \
	}
/* <-- specialization for array of pointers */

#define array_generator(T, name) \
	typedef struct name { \
		T *data; \
		int len; \
		int capacity; \
		\
		int (*push)(struct name *arr, T *item); \
		int (*pop_by_ind)(struct name *arr, int pos); \
		T *(*at)(struct name *arr, int pos); \
		int (*comparator)(const void *item1, const void *item2); \
		int (*pop)(struct name *arr, T *addr); \
		T *(*find)(struct name *arr, T *item, int pos); \
		int (*push_array)(struct name *arr, T *from, int len); \
		\
		/* special functions for array of pointers */ \
		int (*push_p)(struct name *arr, T item); \
		T (*at_p)(struct name *arr, int pos); \
		int (*pop_p)(struct name *arr, T addr); \
		T (*item_constructor_p)(); \
		void (*item_destructor_p)(T item); \
		T (*find_p)(struct name *arr, T item, int pos); \
	} name##_t; \
	\
	__array_new_gen(T, name)

#endif // __ARRAY_H__
