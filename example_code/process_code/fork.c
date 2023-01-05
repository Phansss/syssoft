#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "errmacros.h"

#define LOOPS	5
#define CHILD_POS	"\t\t\t"

void final_message(void) 
{
  pid_t pid = getpid();
  printf("Process %d is now exiting ...\n", pid);
}

void run_child (void)
{
  pid_t my_pid, parent_pid; 
  my_pid = getpid();
  parent_pid = getppid();
  int i = LOOPS;
  
  while ( i-- )
  {
    printf(CHILD_POS "Child process (pid = %d) of parent (pid = %d) ...\n", my_pid, parent_pid);
    sleep(1);
  } 
  exit(EXIT_SUCCESS); // this terminates the child process
}

int main( void ) 
{
  pid_t my_pid, child_pid; 
  
  atexit( final_message );
  my_pid = getpid();
  printf("Parent process (pid = %d) is started ...\n", my_pid);
  
  child_pid = fork();
  SYSCALL_ERROR(child_pid);
  
  if ( child_pid == 0  )
  {  
    run_child();
  }
  else
  {  
    // parent’s code
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid);
    child_pid = fork();
    SYSCALL_ERROR(child_pid);
    if ( child_pid == 0  )
    {  
      run_child();
    }
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid);
  }
  
  printf("Parent process is going to sleep a bit ...\n");
  sleep(LOOPS+1); // notice: sequential execution of both child process would require LOOPS+LOOPS sleep time, concurrent execution requires less than LOOPS+LOOPS sleep time!
  sleep(100); // use this to show <defunct> process with ps
  exit(EXIT_SUCCESS);
}

