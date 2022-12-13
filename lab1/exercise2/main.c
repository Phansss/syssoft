#include <stdio.h>
#include <limits.h>

typedef enum {
    ALLOCATED, FREE
} mem_status;

mem_status ms = FREE;
int x;
float f;
double d;
int *ptr_i = &x;
float *ptr_f = &f;
double *ptr_d = &d;
short s;
long l;
//To check whether it is signed or unsigned, place 'ç' instead of 5 in the next three variable declarations.
//Do this because ç is part of the extended ASCII table. 
signed char sc = '5';
unsigned char uc = '5';
char c = '5';

int main() {
	printf("The sizes are:\n memstatus %ld\n int %ld\n float %ld\n double %ld\n pointer_int %ld\n pointer_double %ld\n pointer_float %ld\n short %ld\n long %ld\n signed char %ld\n unsigned char %ld\n",sizeof(ms),sizeof(x),sizeof(f),sizeof(d), sizeof(ptr_i)
		, sizeof (ptr_d), sizeof(ptr_f), sizeof(s), sizeof(l), sizeof(sc), sizeof(uc));
	/*	
		From the output we can see that the length of an int is doubled or halved by the types 'long' and 'short'.
		Note: the sizeof() function returns the number of bytes a data type consists of in memory.

		OUTPUT:
			int 4
 			float 4
 			double 8
 			pointer_int 8
 			pointer_double 8
 			pointer_float 8
 			short 2
			long 8
 			signed char 1
 			unsigned char 1
 	*/




	//Checking whether char is signed or unsigned. 
	printf("\nThe integer value of the signed char %c is %d.\nThe integer value of the unsigned char %c is %d.\nThe integer value of char %c is %d\n.",
		sc, sc, uc, uc, c, c);
	/* This check failed to compile because the compiler gives an error when assigning values from the extended 
		ASCCII table to the (signed or unsigned) char type. However, from the error message it is clear that the char type
		is signed by deafault since the value of -89 is unsuccesfully assigned to c. 
		
		ERROR MESSAGE: 
			Sizeof.c:12:18: error: multi-character character constant [-Werror=multichar]
   				12 | signed char sc = 'ç';
     			   |                  ^~~~
			Sizeof.c:12:18: error: overflow in conversion from ‘int’ to ‘signed char’ changes value from ‘50087’ to ‘-89’ [-Werror=overflow]
			Sizeof.c:13:20: error: multi-character character constant [-Werror=multichar]
   				13 | unsigned char uc = 'ç';
     			   |                    ^~~~
			Sizeof.c:13:20: error: unsigned conversion from ‘int’ to ‘unsigned char’ changes value from ‘50087’ to ‘167’ [-Werror=overflow]
			Sizeof.c:14:10: error: multi-character character constant [-Werror=multichar]
		    	14 | char c = 'ç';
  		    	   |          ^~~~
			Sizeof.c:14:10: error: overflow in conversion from ‘int’ to ‘char’ changes value from ‘50087’ to ‘-89’ [-Werror=overflow]
			cc1: all warnings being treated as errors
	*/

	//This is a test for git
	}
