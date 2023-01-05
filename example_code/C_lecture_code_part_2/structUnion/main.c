

#include <stdio.h>

int main(int argc, char **argv)
{
  /*
   *	printf("%u\n", 13u);
   *	printf("%ld\n", 12345L);
   *	printf("%lld\n", 12345678910LL);
   *	printf("%Lg\n", 1.2L);
   *	printf("\\o%o\n", 077u);
   *	printf("0x%0x\n", 0x7B2F);
   */
  
  
  int x, y;
  x = 10;
  y = 20;
  x ^= y;
  y ^= x;
  x ^= y;
  printf("x = %d \t y = %d\n", x, y);
  
  
  
  /*
   *	struct byte {
   *		unsigned char b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1, b8:1; 
};

struct byte by = {0};

by.b1 = 1;
by.b3 = 1; 

printf("%d\n", *(int *)&by);
*/
  
  struct bits {
    unsigned char b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1, b8:1; 
  };
  
  union myByte {
    unsigned char byte;
    struct bits bit;
  };
  
  union myByte by;
  
  //printf("%ld\n", sizeof(by));
  
  by.byte = 0xFF;
  printf("%d\n", by.byte);
  
  by.bit.b1 = 0;
  by.bit.b2 = 0; 
  //by.bit.b3 = 0; 
  //by.bit.b4 = 0;
  //by.bit.b5 = 0; 
  //by.bit.b6 = 0; 
  //by.bit.b7 = 0; 
  //by.bit.b8 = 0; 
  
  
  printf("%d\n", by.byte);
  
  return 0;
}

