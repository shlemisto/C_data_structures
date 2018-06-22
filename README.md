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

array_generator(data_t, data_array); // or array_generator(struct data, data_array)
array_generator(int, int_array);

int data_comparator(data_t *item1, data_t *item2)
{
	return (item1->a == item2->a) && (item1->b == item2->b);
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
	array_pop(iarr, -1);

	array_for_each_val(iarr, val)
		printf("arr[%d] = %d\n", __aind(val), val);

	array_free(iarr);

	printf("\n===============\n\n");

	darr = array_new(data_array);
	if (!darr) {
		printf("no memory");
		return ENOMEM;
	}

	array_set_comparator(darr, data_comparator);

	data_t d;
	d.a = 231;
	d.b = -23.084124;

	array_push_val(darr, ((data_t) { .a = 1, .b = 0.221 }));
	array_push_val(darr, ((data_t) { .a = 5, .b = 2.123 }));
	array_push_val(darr, ((data_t) { .a = 23, .b = -2.123 }));
	array_push(darr, &d);

	array_for_each(darr, iter)
		printf("a = %d, b = %f\n", iter->a, iter->b);

	data_t *at = array_at(darr, -1);
	printf("darr[-1] = { a = %d, b = %f }\n", at->a, at->b);

	int ind = array_find_val(darr, ((data_t) { .a = 23, .b = -2.0 }));
	printf("%s\n", ind < 0 ? "not found" : "found");
	ind = array_find(darr, &d);
	printf("%s\n", ind < 0 ? "not found" : "found");

	array_free(darr);

	return 0;
}
```

To compile:
```
gcc -o array array.c
```
