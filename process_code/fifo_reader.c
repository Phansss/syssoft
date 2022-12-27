#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <stdio.h> 
#include "errmacros.h"

#define FIFO_NAME 	"MYFIFO" 
#define MAX 		80

int main(void) { // FIFO reader
  FILE *fp; 
  int result;
  char *str_result;
  char recv_buf[MAX]; 
  
  /* Create the FIFO if it does not exist */ 
  result = mkfifo(FIFO_NAME, 0666);
  CHECK_MKFIFO(result); 
  
  fp = fopen(FIFO_NAME, "r"); //No matter which process starts first, the other one will block on fopen()
  printf("syncing with writer ok\n"); 
  FILE_OPEN_ERROR(fp);

  do 
  {
    str_result = fgets(recv_buf, MAX, fp);
    if ( str_result != NULL )
    { 
      printf("Message received: %s", recv_buf); 
    }
  } while ( str_result != NULL ); 

  
  result = fclose( fp );
  FILE_CLOSE_ERROR(result);
  
  exit(EXIT_SUCCESS);
}



