

#include <time.h>
#include <stdio.h>

// compile this code with gcc -02 to show difference between volatile / non-volatile


int main(void)
{
  time_t start,end;
  volatile long unsigned t;
  //long unsigned t;
  
  start = time(NULL);
  t = 0; 
  while ( t < 1000000000 ) 
  {
    // update t
    t++;
  }
  end = time(NULL);
  printf("Loop used %f seconds.\n", difftime(end, start));
  return 0;
}
=