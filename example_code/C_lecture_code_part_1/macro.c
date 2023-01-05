#include <stdio.h>
#include <stdlib.h>


// attempt 1
//#define FREE(p) free(p); p=NULL

// attempt 2
// #define FREE(p) {free(p); p=NULL;}

// attempt 3
// #define FREE(p) do { free(p);p=NULL;} while(0)


int main()
{
  int * ptr = malloc( 10*sizeof(int) );
  
  if ( 1 ) // run code with condition true and false, and explain the result
      FREE(ptr);
  else
      printf("do other stuff here\n");
}

