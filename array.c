#include <stdio.h>
#include "array.h"

#ifndef ARRAY_LEN
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

typedef struct data {
	int a;
	float b;
	void *pad;
} data_t;

parray_generator(struct data *, data_array)
array_generator(int, int_array)

int data_comparator(const void *v1, const void *v2)
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
	if (!d)
		return;
	free(d->pad);
	free(d);
}

data_t *data_constructor()
{
	data_t *p = calloc(1, sizeof(data_t));
	if (!p)
		return NULL;
	if (!(p->pad = calloc(1, 10))) {
		free(p);
		return NULL;
	}

	p->a = 0.1;
	p->b = 1.0;

	return p;
}

int main(void)
{
	struct data_array *darr = NULL;
	int_array_t *iarr = NULL;
	int values[] = { 1, 2, 5, 8, -1, 44, 3, 0, 66 };
	int i = 54;

	iarr = array_new(int_array, int_comparator);
	if (!iarr) {
		printf("no memory");
		return ENOMEM;
	}

	array_push(iarr, &i);
	array_push_array(iarr, values, ARRAY_LEN(values));

	array_pop_by_ind(iarr, 0);
	array_pop_by_ind(iarr, 0);

	qsort(array_data(iarr), array_len(iarr), sizeof(int), int_comparator);

	array_for_each(val, iarr)
		printf("arr[%d] = %d\n", __aind(val), *val);

	array_free(iarr);

	printf("\n===============\n\n");

	darr = parray_new(data_array, data_constructor, data_destructor, data_comparator);
	if (!darr) {
		printf("no memory");
		return ENOMEM;
	}

	for (i = 0; i < 10; ++i) {
		// allocate memory with predefined item value
		data_t *pd = parray_item_alloc(darr);
		if (pd)
			parray_push(darr, pd);
	}

	data_t **ppd = (data_t **) calloc(10, sizeof(data_t));
	if (ppd) {
		int flag = 1;
		for (i = 0; i < 10; ++i) {
				ppd[i] = (data_t *) calloc(1, sizeof(data_t));
				if (ppd[i]) {
					ppd[i]->a = i;
					ppd[i]->b = -i*i;
					ppd[i]->pad = NULL;
				} else
					flag = 0;
		}

		if (flag)
			parray_push_array(darr, ppd, 10);
		else {
			for (i = 0; i < 10; ++i)
				free(ppd[i]);
		}
		free(ppd);
	}

	parray_pop_by_ind(darr, 0);
	parray_pop(darr, parray_at(darr, -1));

	/*qsort(parray_data(darr), parray_len(darr), sizeof(data_t *), data_comparator);*/
	
	/*parray_purge(darr);*/

	parray_for_each(pd, darr) {
		printf("%d %f\n", pd->a, pd->b);
	}

	data_t dd = { .a = -23, .b = 1.34 };
	if (!parray_find(darr, &dd))
		printf("not found\n");
	if (parray_find(darr, parray_at(darr, -1)))
		printf("found\n");

	parray_free(darr);

	return 0;
}
