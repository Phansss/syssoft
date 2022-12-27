#define _GNU_SOURCE 

#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "errmacros.h"

#define MAX 80
#define SHELL "\nhello>"

int spawn (char* program) 
{
  pid_t child_pid;
  
  child_pid = fork();
  SYSCALL_ERROR( child_pid );
  
  if ( child_pid != 0)
  {
    return child_pid;  // return to parent code
  }
  else 
  {
    char* arg_list[] = { program, NULL };
    execvp (program, arg_list);  // run subprogram
    // should never come here ...
    perror("\nAn error occurred in execvp");
    exit(EXIT_FAILURE);
  }
}


int main( int argc, char *argv[] )
{
  char prog_name[MAX+1] = "";
  char *format_spec;
  pid_t child_pid;
  int child_exit_status;
  
  ASPRINTF_ERROR( asprintf( &format_spec, "%%%ds", MAX) ); // create "%MAXs" format spec 
  
  printf("\nStarting my shell ... \n");
  
  while ( 1 ) 
  {
    printf(SHELL);
    
    if ( scanf(format_spec, prog_name) == 1 ) // input argument read
    {
      if ( strcmp( prog_name, "bye" ) == 0 )
      {
	printf("\nbye bye ...\n");
	break;
      }
      child_pid = spawn ( prog_name );
      SYSCALL_ERROR( waitpid(child_pid, &child_exit_status, 0) );
    }
  }
  
  free( format_spec );
  exit(EXIT_SUCCESS);
}

