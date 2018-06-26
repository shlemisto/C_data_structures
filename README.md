# c_array
Simple array implementation in C.

Code example:
```
#include <stdio.h>
#include "array.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct data {
	int a;
	float b;
} data_t;

parray_generator(struct data *, data_array);
array_generator(int, int_array);

int pdata_comparator(const void *v1, const void *v2)
{
	data_t **item1 = (data_t **) v1;
	data_t **item2 = (data_t **) v2;

	return (*item1)->a - (*item2)->a;
}

int int_comparator(const void *v1, const void *v2)
{
	int *item1 = (int *) v1;
	int *item2 = (int *) v2;

	return *item1 - *item2;
}

void data_destructor(data_t *d)
{
	free(d);
}

int main(void)
{
	int_array_t *iarr = NULL; // or struct int_array *iarr = NULL;
	struct data_array *darr = NULL; // or data_array_t *darr = NULL;
	int values[] = { 1, 2, 5, 8, -1, 44, 3, 0, 66 };
	int i = 54;

	iarr = array_new(int_array);
	if (!iarr) {
		printf("no memory");
		return ENOMEM;
	}

	array_set_comparator(iarr, int_comparator);

	array_push_val(iarr, -22);
	array_push(iarr, &i);
	array_push_array(iarr, values, ARRAY_LEN(values));

	array_pop(iarr, 0);
	array_pop(iarr, 0);

	//qsort(array_data(iarr), array_len(iarr), sizeof(int), int_comparator);

	array_for_each_val(iarr, val)
		printf("arr[%d] = %d\n", __aind(val), val);

	array_free(iarr);

	printf("\n===============\n\n");

	darr = parray_new(data_array);
	if (!darr) {
		printf("no memory");
		return ENOMEM;
	}

	parray_set_comparator(darr, pdata_comparator);
	parray_set_item_destructor(darr, data_destructor);

	for (i = 0; i < 10; ++i) {
		data_t *pd = (data_t *) malloc(sizeof(data_t));
		if (pd) {
			pd->a = i*i;
			pd->b = i + 1.34;

			parray_push(darr, pd);
		}
	}

	parray_pop(darr, 0);
	parray_pop(darr, -2);
	parray_pop(darr, -1);

	for (i = 0; i < 10; ++i) {
		data_t *pd = (data_t *) malloc(sizeof(data_t));
		if (pd) {
			pd->a = 2*i;
			pd->b = -i + 1.34;

			parray_push(darr, pd);
		}
	}

	//qsort(parray_data(darr), parray_len(darr), sizeof(data_t *), pdata_comparator);

	parray_for_each(darr, pd)
		printf("%d %f\n", pd->a, pd->b);

	data_t dd = { .a = -3, .b = 1 };
	if (!parray_find(darr, &dd));
		printf("not found\n");
	if (parray_find(darr, parray_at(darr, -1)));
		printf("found\n");

	parray_free(darr);

	return 0;
}
```

To compile:
```
gcc -o array array.c
```
