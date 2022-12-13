#include <stdio.h>


void swap_pointers(int **w, int **x) {
	int* i = *w;
	*w = *x;
	*x = i;
	
}


int main() {
	int a = 1;
	int b = 2;
	int *p = &a;
	int *q = &b;

	printf("BEFORE SWAPPOINTERS: address of a = %p and b = %p\n", p, q);
	swap_pointers(&p, &q);
	printf("AFTER SWAPPOINTERS: address of a = %p and b = %p\n", p, q);

}