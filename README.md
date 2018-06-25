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

int data_comparator(data_t *item1, data_t *item2)
{
	return (item1->a == item2->a) && (item1->b == item2->b);
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

	array_push_val(iarr, -22);
	array_push(iarr, &i);
	array_push_array(iarr, values, ARRAY_LEN(values));

	array_pop(iarr, 0);
	array_pop(iarr, 0);

	array_for_each_val(iarr, val)
		printf("arr[%d] = %d\n", __aind(val), val);

	array_free(iarr);

	printf("\n===============\n\n");

	darr = parray_new(data_array);
	if (!darr) {
		printf("no memory");
		return ENOMEM;
	}

	parray_set_comparator(darr, data_comparator);
	parray_set_item_destructor(darr, data_destructor);

	for (i = 0; i < 10; ++i) {
		data_t *pd = (data_t *) malloc(sizeof(data_t));
		if (pd) {
			pd->a = i;
			pd->b = i + 1.34;

			/*printf("alloc %llx\n", (void *) pd);*/
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

			/*printf("alloc %llx\n", (void *) pd);*/
			parray_push(darr, pd);
		}
	}

	parray_for_each(darr, pd)
		printf("%d %f\n", pd->a, pd->b);

	data_t dd = { .a = 0, .b = 1 };
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
