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

int int_comp(int *a, int *b)
{
    return *a - *b;
}

void data_destructor(data_t *d)
{
	if (!d)
		return;

	free(d->str);
	free(d);
}

data_t *data_constructor(int a)
{
	data_t *p = calloc(1, sizeof(data_t));

    if (NULL == p)
		return NULL;
    else if (NULL == (p->str = calloc(1, 256*sizeof(char))))
    {
		free(p);
		return NULL;
	}

    p->a = a;
    sprintf(p->str, "int as string %d", p->a);

	return p;
}

array_generator(int, int_arr, NULL, NULL, int_comp)
parray_generator(data_t *, data_arr, data_constructor, data_destructor, NULL)
map_generator(char *, data_t *, data_map, data_constructor, data_destructor, string_comp)

int main(void)
{
    int sts = ERR_OK;
    data_map_t *map = NULL;
    data_arr_t *arr = NULL;
    int_arr_t *iarr = NULL;
    data_t *d = NULL;
    map_iter(map) *iter;

    srand(time(0));

    if (NULL == (map = map_new(data_map)) ||
        NULL == (arr = parray_new(data_arr)) ||
        NULL == (iarr = array_new(int_arr)))
    {
        sts = ERR_NO_MEM;
        goto exit;
    }

    for (int i = 0; i < 10; ++i)
    {
        int err = ERR_OK;
        data_t *item = NULL;
        char key[32] = { 0 };

        sprintf(key, "%d", rand() % 100);
        item = map_val_new(map, i);

        if (ERR_OK != (err = map_push(map, key, item)))
            map_val_free(map, item);

        item = parray_val_new(arr, i*i);
        if (ERR_OK != (err = parray_push(arr, item)))
            parray_val_free(arr, item);

        array_push(iarr, (&(int) { i*i*i }));
    }

    printf("------ begin int array -------\n");

    int i;
    array_for_each_val(iarr, i)
        printf("%d\n", i);

    {
        int *pi = array_find(iarr, (&(int) { 54 }));
        if (NULL == pi)
            printf("54 is not found!\n");
        pi = array_find(iarr, (&(int) { 512 }));
        if (pi)
            printf("512 is found!\n");
    }

    printf("------ end int array -------\n\n");

    printf("------ begin array -------\n");

    parray_for_each(arr, d)
        printf("{ %d; %s }\n", d->a, d->str);

    printf("------ end array -------\n\n");

    printf("------ begin map -------\n\n");

    map_for_each(map, iter)
        printf("%s: { %d; %s}\n", map_key(iter), map_val(iter)->a, map_val(iter)->str);

    printf("\n------ delete some items from map -------\n");

    map_for_each_safe(map, node, iter)
    {
        if (atoi(map_key(iter)) % 2)
            map_pop_safe(map, node);
    }

    map_for_each(map, iter)
        printf("%s: { %d; %s}\n", map_key(iter), map_val(iter)->a, map_val(iter)->str);

    printf("------ end map -------\n");


exit:
    array_free(iarr);
    parray_free(arr);
    map_free(map);

    return sts;
}
