#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h> 
#include "errmacros.h"

#define FIFO_NAME 	"MYFIFO" 
#define MAX 		80
#define LOOPS		5

int main(void) { // FIFO reader
  FILE *fp; 
  int result;
  char *send_buf; 
  int i = LOOPS;
  
  /* Create the FIFO if it does not exist */ 
  result = mkfifo(FIFO_NAME, 0666);
  CHECK_MKFIFO(result); 	
  
  fp = fopen(FIFO_NAME, "w"); // open for writing
  printf("syncing with reader ok\n");
  FILE_OPEN_ERROR(fp);
  
  while ( i-- )
  {
    asprintf( &send_buf, "Test message %d\n", LOOPS-i );
    if ( fputs( send_buf, fp ) == EOF ) // Looks like text-based IO but actually is FIFO
    {
      fprintf( stderr, "Error writing data to fifo\n");
      exit( EXIT_FAILURE );
    } 
    FFLUSH_ERROR(fflush(fp)); // Make sure that the system sends everything to the FIFO. (nothing stays in buffer)
    printf("Message send: %s", send_buf); 
    free( send_buf );
    sleep(1); //wait a moment to make sure that everything is flushed.
  } 
  
  result = fclose( fp );
  FILE_CLOSE_ERROR(result);
  
  exit(EXIT_SUCCESS);
}





