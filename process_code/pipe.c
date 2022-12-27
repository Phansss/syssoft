#define _GNU_SOURCE

#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "errmacros.h"

#define LOOPS		5
#define MAX			80
#define CHILD_POS	"\t\t\t"
// NOTE : IMPOSSIBLE TO WRITE TO CLOSED PIPE 5FROM READING SIDE).
void run_child ( int pfds[], int exit_code ) // Give both file descriptor
{
  pid_t my_pid, parent_pid; 
  my_pid = getpid();
  parent_pid = getppid();
  int i = LOOPS;
  char *send_buf;
  
  close(pfds[0]); // Reading fs not needed by child
  printf(CHILD_POS "Greetings from child process (pid = %d) of parent (pid = %d) ...\n", my_pid, parent_pid);	
  while ( i-- )
  {
    asprintf( &send_buf, "Test message %d", LOOPS-i );
    write(pfds[1], send_buf, strlen(send_buf)+1 ); // don't forget to send \0!
    free( send_buf );
    sleep(1);
  } 
  close(pfds[1]); // indicate end of writing
  printf(CHILD_POS "Child process (pid = %d) of parent (pid = %d) is terminating ...\n", my_pid, parent_pid);	
  exit( exit_code ); // this terminates the child process
}

int main( void ) 
{
  pid_t my_pid, child_pid;  //Define pipe ID
  int pfds[2];              // Pipe file descriptors (to acess the pipe)
  int result;
  
  my_pid = getpid();        // Get process ID of calling process
  printf("Parent process (pid = %d) is started ...\n", my_pid); // Parent process with process ID.
  
  result = pipe( pfds );     // Create a pipe
  SYSCALL_ERROR( result );   // Error handling
  
  child_pid = fork();       // Create child process. If current process == parent process, child_pid is the pid of the newly created process.
                            //                       If current process == child process, child_pid is 0.
  SYSCALL_ERROR(child_pid); // ERROR Handling
  
  if ( child_pid == 0  )
  {  
    run_child( pfds, 0 );
  }
  else
  {  
    // parent’s code
    char recv_buf[MAX]; // Create receiving buffer.
    
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid);
    close( pfds[1] ); // Writing file descriptor not needed by parent
    
    do 
    {
      result = read(pfds[0], recv_buf, MAX); // wait until data are available and read them in rx buff. Returns 0 when EOF (pipe closed). 
      SYSCALL_ERROR( result );
      if ( result > 0 )
      { 
	printf("Message received from child %d: \"%s\"\n", child_pid, recv_buf); 
      }
    } while ( result > 0 ); // 0 means END OF FILE. Only returned when pipe is closed from the other side. 
                            // read() on line 67 simply waits if there are no bytes available. (BLOCKING -> 'by default all IO is blocking').
    
    // pipe closed by child: no more reading
    close( pfds[0] );
    
    // wait on termination of child process
    waitpid(child_pid, NULL, 0);
    
    printf("Parent process (pid = %d) is terminating ...\n", my_pid);
  }
  
  exit(EXIT_SUCCESS);
}





