#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "array.h"
#include "list.h"
#include "map.h"

#ifndef ARRAY_LEN
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

typedef struct data {
	int a;
	char *str;
} data_t;

typedef struct data_key {
	char key[64];
} data_key_t;

int data_comparator(data_t *d1, data_t *d2)
{
	return d1->a - d2->a;
}

int key_comparator(data_key_t k1, data_key_t k2)
{
	return strcmp(k1.key, k2.key);
}

int string_comp(char *k1, char *k2)
{
	return strcmp(k1, k2);
}

void data_destructor(data_t *d)
{
	if (!d)
		return;

	free(d->str);
	free(d);
}

void s_data_destructor(data_t *d)
{
	if (!d)
		return;

	free(d->str);
}

data_t *data_constructor()
{
	data_t *p = calloc(1, sizeof(data_t));

	if (!p)
		return NULL;
	else if (!(p->str = malloc(32))) {
		free(p);
		return NULL;
	}

	p->a = rand() % 50;
	sprintf(p->str, "sq %d", p->a * p->a);

	return p;
}

array_generator(data_t, s_data_array, s_data_destructor, data_comparator)
parray_generator(data_t *, data_array, data_constructor, data_destructor, data_comparator)
list_generator(data_t *, data_list, data_constructor, data_destructor, data_comparator)
map_generator(data_key_t, data_t *, data_map, data_constructor, data_destructor, key_comparator)
map_generator(char *, data_t *, data_map2, data_constructor, data_destructor, string_comp)

int main(void)
{
	int rv = 0;
	data_array_t *arr = NULL;
	s_data_array_t *s_arr = NULL;
	data_list_t *list = NULL;
	data_map_t *map = NULL;
	data_map2_t *map2 = NULL;
	data_t *data = NULL;
	map_key_val(map) *kv = NULL;
	map_key_val(map2) *kv2;
	data_t *find_in_map = NULL;

	srand(time(NULL));

	s_arr = array_new(s_data_array);
	arr = parray_new(data_array);
	list = list_new(data_list);
	map = map_new(data_map);
	map2 = map_new(data_map2);

	if (!s_arr || !arr || !list || !map || !map2) {
		printf("no memory\n");
		rv = ENOMEM;
		goto exit;
	}

	for (int i = 0; i < 50; ++i) {
		data_t d;

		d.a = 50-i;
		d.str = calloc(1, 10);
		if (d.str) {
			sprintf(d.str, "%d", d.a);
			if (array_push(s_arr, &d))
				array_item_destroy(s_arr, &d);
		}
	}

	for (int i = 0; i < 50; ++i) {
		data_t *item = parray_new_item(arr);
		if (parray_push(arr, item))
			parray_item_destroy(arr, item);
	}

	for (int i = 0; i < 50; ++i) {
		data_t *item = list_new_item(list);
		if (list_push(list, item))
			list_item_destroy(list, item);
	}

	for (int i = 0; i < 50; ++i) {
		data_key_t key = { 0 };
		data_t *val = map_new_val(map);
		if (val) {
			sprintf(key.key, "data_key_t %d", rand() % 20);
			if (map_push(map, key, val))
				map_item_destroy(map, val);
		}
	}

	for (int i = 0; i < 50; ++i) {
		char s[100] = { 0 };
		data_t *val = map_new_val(map);
		sprintf(s, "string %d", rand() % 25);
		if (map_push(map2, s, val))
			map_item_destroy(map, val);
	}

	if (array_pop_by_ind(s_arr, -1))
		printf("unable to pop the last elem from s_arr!\n");

	if (parray_pop_by_ind(arr, 0))
		printf("unable to pop the first elem from arr!\n");

	if (parray_pop(arr, &((data_t) { .a = 4 })))
		printf("unable to pop '4' from arr; not found in arr\n");

	if (list_pop(list, &((data_t) { .a = 11 })))
		printf("unable to delete '11' from list; not found in list\n");

	find_in_map = map_find(map, ((data_key_t) { .key = "data_key_t 10" }));
	if (find_in_map) {
		printf("found { data_key_t 10: [%d, %s] } delete it!\n", find_in_map->a, find_in_map->str);
		map_pop(map, ((data_key_t) { .key = "data_key_t 10" }));
	}

	find_in_map = map_find(map2, ((char *) "string 10"));
	if (find_in_map) {
		printf("found { string 10: [%d, %s] } delete it!\n", find_in_map->a, find_in_map->str);
		map_pop(map2, ((char *) "string 10"));
	}

	printf("\n\ns_arr\n");
	array_for_each(s_arr, data)
		printf("[%d, %s], ", data->a, data->str);
	printf("\n\n");

	printf("arr\n");
	parray_for_each(arr, data)
		printf("[%d, %s], ", data->a, data->str);
	printf("\n\n");

	printf("list\n");
	list_for_each(list, data)
		printf("[%d, %s], ", data->a, data->str);
	printf("\n\n");

	printf("map\n");
	map_for_each(map, kv)
		printf("{%s: [%d, %s]}\n", map_key(kv).key, map_val(kv)->a, map_val(kv)->str);
	printf("\n");

	printf("map2\n");
	map_for_each(map2, kv2)
		printf("{%s: [%d, %s]}\n", map_key(kv2), map_val(kv2)->a, map_val(kv2)->str);
	printf("\n");

exit:
	array_free(s_arr);
	parray_free(arr);
	list_free(list);
	map_free(map);
	map_free(map2);

	return rv;
}
