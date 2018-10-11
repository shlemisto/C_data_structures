#ifndef __ARRAY_H__
#define __ARRAY_H__

/*
 * array for containing objects
 * parray it's specialization of array for containing pointers to object
*/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

// get index from iterator
#define __aind(name) __i_ ## name

static inline void __do_nothing_array() {}

#define __array_push_array(name)  __arr_## name ##_push_array
#define __array_push(name)	 __arr_## name ##_push
#define __array_pop(name)	 __arr_## name ##_pop
#define __array_pop_by_ind(name) __arr_## name ##_pop_by_ind
#define __array_at(name)	 __arr_## name ##_at
#define __array_find(name)	 __arr_## name ##_find
#define __array_new(name)	 __arr_## name ##_new
#define __array_purge(name)	 __arr_## name ##_purge
#define __array_free(name)	 __arr_## name ##_free

#define array_new(name) __array_new(name)()
#define array_item_destroy(arr, item) arr->item_destructor ? arr->item_destructor(item) : __do_nothing_array()
#define array_len(arr) arr->len
#define array_data(arr) arr->data
#define array_is_empty(arr) (arr->len == 0)
#define array_at(arr, i) arr->at(arr, i)
#define array_at_val(arr, i) *arr->at(arr, i) // !!! be careful
#define array_pop(arr, pos) arr->pop(arr, pos)
#define array_pop_by_ind(arr, pos) arr->pop_by_ind(arr, pos)
#define array_push(arr, item) arr->push(arr, item)
#define array_push_array(arr, from, len) arr->push_array(arr, from, len)
#define array_find_from(arr, what, pos) arr->find(arr, what, pos)
#define array_find(arr, what) array_find_from(arr, what, 0)
#define array_find_val(arr, val) array_find_val_from(arr, val, 0)
#define array_free(arr) if (arr) arr->free(arr)
#define array_purge(arr) arr->purge(arr)

// note: pos >= 0
#define __array_for_each(arr, iter, pos) \
	for (int __aind(iter) = (pos); ((pos) >= 0) && (__aind(iter) < array_len(arr)) && (iter = array_at(arr, __aind(iter)), 1); ++__aind(iter))
#define array_for_each(arr, iter) \
	__array_for_each(arr, iter, 0)
#define array_for_each_val(arr, iter) \
	for (int __aind(iter) = 0 ; __aind(iter) < array_len(arr) && (iter = array_at_val(arr, __aind(iter)), 1); ++__aind(iter))

#define __array_new_gen(T, name, __destructor, __comparator) \
	\
	static void __array_purge(name)(struct name *arr) \
	{ \
		if (arr->item_destructor) { \
			for (int i = 0; i < array_len(arr); ++i) \
				arr->item_destructor(array_at(arr, i)); \
		} \
		\
		arr->len = 0; \
		arr->capacity = 10; \
	} \
	\
	static void __array_free(name)(struct name *arr) \
	{ \
		if (arr->item_destructor) { \
			for (int i = 0; i < array_len(arr); ++i) \
				arr->item_destructor(array_at(arr, i)); \
		} \
		free(arr->data); \
		free(arr); \
		arr = NULL; \
	} \
	\
	static int __array_push(name)(struct name *arr, T *item) \
	{ \
		T *tmp_arr = NULL; \
		\
		if (!item) \
			return EPERM; \
		\
		if (arr->capacity == arr->len) { \
			arr->capacity *= 2; \
			\
			tmp_arr = (T *) realloc(arr->data, arr->capacity * sizeof(T)); \
			if (!tmp_arr) { \
				arr->free(arr); \
				return ENOMEM; \
			} \
			\
			arr->data = tmp_arr; \
		} \
		\
		memcpy((void *) &arr->data[arr->len++], (void *) item, sizeof(T)); \
		\
		return 0; \
	} \
	\
	static int __array_push_array(name)(struct name *arr, T *from, int len) \
	{ \
		int ret = 0; \
		\
		if (!from) \
			return EPERM; \
		\
		for (int i = 0 ; i < len; ++i) { \
			if ((ret = __array_push(name)(arr, &from[i]))) \
			    break; \
		} \
		return ret; \
	} \
	\
	static T *__array_at(name)(struct name *arr, int pos) \
	{ \
		if (pos >= 0) \
			return pos < array_len(arr) ? &arr->data[pos] : NULL; \
		else \
			return pos >= -array_len(arr) ? &arr->data[array_len(arr)+pos] : NULL; \
	} \
	\
	static int __array_pop_by_ind(name)(struct name *arr, int pos) \
	{ \
		T *addr = NULL; \
		\
		if (array_is_empty(arr)) \
			return ENODATA; \
		else if ((pos >= array_len(arr)) || (pos < -array_len(arr))) \
			return EPERM; \
		\
		addr = __array_at(name)(arr, pos); \
		/* case for len == 1, or for the last elem */ \
		if ((pos == array_len(arr) - 1) || (pos == -1)) \
			arr->len -= 1; \
		else if ((pos >= 0) && (pos < array_len(arr) - 1)) { \
			for (int i = pos; i < array_len(arr) - 1; ++i) \
				memcpy((void *) &arr->data[i], (void *) &arr->data[i+1], sizeof(T)); \
			arr->len -= 1; \
		} else if ((pos < 0) && (pos >= -array_len(arr))) { \
			for (int i = array_len(arr) + pos; i < array_len(arr) - 1; ++i) \
				memcpy((void *) &arr->data[i], (void *) &arr->data[i+1], sizeof(T)); \
			arr->len -= 1; \
		} \
		if (arr->item_destructor) \
			arr->item_destructor(addr); \
		\
		return 0; \
	} \
	\
	static int __array_pop(name)(struct name *arr, T *addr) \
	{ \
		int ret = ENODATA; \
		__typeof(arr->data) iter = NULL; \
		\
		if (!addr || !arr->comparator) \
			return EPERM; \
		\
		array_for_each(arr, iter) { \
			if (0 == arr->comparator(iter, addr)) { \
				ret = __array_pop_by_ind(name)(arr, __aind(iter)); \
				break; \
			} \
		} \
		\
		return ret; \
	} \
	\
	static T *__array_find(name)(struct name *arr, T *what, int pos) \
	{ \
		T *iter = NULL; \
		\
		if (!what || !arr->comparator) \
			return NULL; \
		\
		__array_for_each(arr, iter, pos) { \
			if (0 == arr->comparator(iter, what)) \
				return iter; \
		} \
		\
		return NULL; \
	} \
	\
	static struct name *__array_new(name)(void) \
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
		arr->comparator = __comparator; \
		arr->item_destructor = __destructor; \
		arr->pop_by_ind = __array_pop_by_ind(name); \
		arr->free = __array_free(name); \
		arr->purge = __array_purge(name); \
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
#define __parray_free(name)	  __parr_## name ##_free
#define __parray_purge(name)	  __parr_## name ##_purge

#define parray_new(name) __parray_new(name)()
#define parray_len(arr) arr->len
#define parray_data(arr) arr->data
#define parray_is_empty(arr) (arr->len == 0)
#define parray_set_item_destructor(arr, d) arr->item_destructor_p = d
#define parray_item_destroy(arr, item) arr->item_destructor_p ? arr->item_destructor_p(item) : __do_nothing_array()
#define parray_set_comparator(arr, c) arr->comparator_p = c
#define parray_push(arr, item) arr->push_p(arr, item)
#define parray_push_array(arr, from, len) arr->push_array(arr, from, len)
#define parray_pop_by_ind(arr, pos) arr->pop_by_ind(arr, pos)
#define parray_pop(arr, addr) arr->pop_p(arr, addr)
#define parray_at(arr, i) arr->at_p(arr, i)
#define __parray_for_each(arr, iter, pos) \
	for (int __aind(iter) = (pos); ((pos) >= 0) && (__aind(iter) < array_len(arr)) && (iter = parray_at(arr, __aind(iter)), 1); ++__aind(iter))
#define parray_for_each(arr, iter) __parray_for_each(arr, iter, 0)
#define parray_find_from(arr, what, pos) arr->find_p(arr, what, pos)
#define parray_find(arr, what) parray_find_from(arr, what, 0)
#define parray_new_item(arr) arr->item_constructor_p ? arr->item_constructor_p() : NULL

#define parray_purge(arr) arr->purge(arr)
#define parray_free(arr) if (arr) arr->free(arr)

#define parray_generator(T, name, __constructor, __destructor, __comparator) \
	array_generator(T, name, NULL, NULL) \
	\
	static void __parray_purge(name)(struct name *arr) \
	{ \
		if (arr) { \
			if (arr->item_destructor_p) { \
				for (int i = 0 ; i < array_len(arr); ++i) \
					arr->item_destructor_p(parray_at(arr, i)); \
			} \
			arr->len = 0; \
			arr->capacity = 10; \
		} \
	} \
	\
	static void __parray_free(name)(struct name *arr) \
	{ \
		if (arr) { \
			if (arr->item_destructor_p) { \
				for (int i = 0 ; i < array_len(arr); ++i) \
					arr->item_destructor_p(parray_at(arr, i)); \
			} \
			\
			free(arr->data); \
			free(arr); \
			arr = NULL; \
		} \
	} \
	\
	static int __parray_push(name)(struct name *arr, T item) \
	{ \
		if (!item) \
			return EPERM; \
		\
		return __array_push(name)(arr, &item); \
	} \
	\
	static T __parray_at(name)(struct name *arr, int pos) \
	{ \
		T *item = __array_at(name)(arr, pos); \
		\
		if (!item) \
			return NULL; \
		\
		return *__array_at(name)(arr, pos); \
	} \
	\
	static int __parray_pop_by_ind(name)(struct name *arr, int pos) \
	{ \
		int ret = ENODATA; \
		T addr = __parray_at(name)(arr, pos); \
		\
		if (addr) { \
			ret = __array_pop_by_ind(name)(arr, pos); \
			if (arr->item_destructor_p) \
				arr->item_destructor_p(addr); \
		} \
		\
		return ret; \
	} \
	\
	static int __parray_pop(name)(struct name *arr, T addr) \
	{ \
		int ret = ENODATA; \
		\
		if (!addr || !arr->comparator_p) \
			return EPERM; \
		\
		__typeof(arr->data[0]) iter = NULL; \
		\
		parray_for_each(arr, iter) { \
			if (0 == arr->comparator_p(iter, addr)) { \
				ret = __parray_pop_by_ind(name)(arr, __aind(iter)); \
				break; \
			} \
		} \
		\
		return ret; \
	} \
	\
	static T __parray_find(name)(struct name *arr, T what, int pos) \
	{ \
		T iter = NULL; \
		\
		if (!arr->comparator_p || !what) \
			return NULL; \
		\
		__parray_for_each(arr, iter, pos) { \
			if (0 == arr->comparator_p(iter, what)) \
				return iter; \
		} \
		\
		return NULL; \
	} \
	\
	static struct name *__parray_new(name)(void) \
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
		arr->comparator_p = __comparator; \
		arr->find_p = __parray_find(name); \
		arr->item_destructor_p = __destructor; \
		arr->item_constructor_p = __constructor; \
		arr->free = __parray_free(name); \
		arr->purge = __parray_purge(name); \
		\
		return arr; \
	}
/* <-- specialization for array of pointers */

#define array_generator(T, name, __constructor, __destructor) \
	typedef struct name { \
		T *data; \
		int len; \
		int capacity; \
		\
		int (*push)(struct name *arr, T *item); \
		int (*pop_by_ind)(struct name *arr, int pos); \
		T *(*at)(struct name *arr, int pos); \
		int (*comparator)(T *item1, T *item2); \
		int (*comparator_p)(T item1, T item2); \
		void (*item_destructor)(T *item); \
		int (*pop)(struct name *arr, T *addr); \
		T *(*find)(struct name *arr, T *item, int pos); \
		int (*push_array)(struct name *arr, T *from, int len); \
		void (*free)(struct name *arr); \
		void (*purge)(struct name *arr); \
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
	__array_new_gen(T, name, __constructor, __destructor)

#endif // __ARRAY_H__
