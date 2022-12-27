#include <sys/types.h>
#include <wait.h>
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

void run_child (int exit_code)
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
  exit(exit_code); // this terminates the child process
}

int main( void ) 
{
  pid_t my_pid, pid, child_pid[2]; 
  int child_exit_status;
  
  atexit( final_message );
  my_pid = getpid();
  printf("Parent process (pid = %d) is started ...\n", my_pid);
  
  child_pid[0] = fork();
  SYSCALL_ERROR(child_pid[0]);
  
  if ( child_pid[0] == 0  )
  {  
    run_child(50);
  }
  else
  {  
    // parent’s code
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid[0]);
    child_pid[1] = fork();
    SYSCALL_ERROR(child_pid[1]);
    if ( child_pid[1] == 0  )
    {  
      run_child(100);
    }
    printf("Parent process (pid = %d) has created child process (pid = %d)...\n", my_pid, child_pid[1]);
  }
  
  // waiting on 1 child
  SYSCALL_ERROR( waitpid(child_pid[1], &child_exit_status,0) );
  if ( WIFEXITED(child_exit_status) )
  {
    printf("Child %d terminated with exit status %d\n", child_pid[1], WEXITSTATUS(child_exit_status));
  }
  else
  {
    printf("Child %d terminated abnormally\n", child_pid[1]);
  }
  
  // waiting on the other child
  SYSCALL_ERROR( waitpid(child_pid[0], &child_exit_status,0) );
  if ( WIFEXITED(child_exit_status) )
  {
    printf("Child %d terminated with exit status %d\n", child_pid[0], WEXITSTATUS(child_exit_status));
  }
  else
  {
    printf("Child %d terminated abnormally\n", child_pid[0]);
  }
  
  exit(EXIT_SUCCESS);
}






